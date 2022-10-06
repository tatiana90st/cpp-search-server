#pragma once
#include "string_processing.h"
#include "document.h"
#include "concurrent_map.h"
#include "log_duration.h"
#include <map>
#include <algorithm>
#include <set>
#include <execution>
#include <utility>
#include <string_view>
#include <string>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double ALMOST_IRRELEVANT = 1e-6;


class SearchServer {
public:
    
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
    {
        if (std::any_of(stop_words_.begin(), stop_words_.end(), [](const std::string_view& word) { return !IsValidWord(word); })) {
            throw std::invalid_argument("Ошибка в стоп-словах");
        }
    }
    
    explicit SearchServer(const std::string& stop_words);

    explicit SearchServer(const std::string_view& stop_words_text);

    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate, typename ExPol>
    std::vector<Document> FindTopDocuments(ExPol& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const;

    template <typename ExPol>
    std::vector<Document> FindTopDocuments(ExPol& policy, const std::string_view& raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const;

    template <typename ExPol>
    std::vector<Document> FindTopDocuments(ExPol& policy, const std::string_view& raw_query) const;

    int GetDocumentCount() const;

    auto begin() const {
        return document_ids_.begin();
    }

    auto end() const {
        return document_ids_.end();
    }

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::sequenced_policy, const std::string_view& raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::execution::parallel_policy, const std::string_view& raw_query, int document_id) const;

    void RemoveDocument(int document_id);

    void RemoveDocument(std::execution::sequenced_policy, int document_id);
    void RemoveDocument(std::execution::parallel_policy, int document_id);

private:
    const std::set<std::string, std::less<>> stop_words_;

    struct DocumentData {
        int rating;
        DocumentStatus status;
        std::string text;
    };

    std::map<int, DocumentData> documents_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::vector<int> document_ids_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;

    static bool IsValidWord(const std::string_view& word);

    bool IsStopWord(const std::string_view& word) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    Query ParseQuery(const std::string_view& text) const;

    double ComputeWordInverseDocumentFreq(const std::string_view& word) const;

    template <typename DocumentPredicate, typename ExPol>
    std::vector<Document> FindAllDocuments(ExPol& policy, Query& query, DocumentPredicate document_predicate) const;
};

template <typename ExPol>
std::vector<Document> SearchServer::FindTopDocuments(ExPol& policy, const std::string_view& raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename ExPol>
std::vector<Document> SearchServer::FindTopDocuments(ExPol& policy, const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}


template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}

template <typename DocumentPredicate, typename ExPol>
std::vector<Document> SearchServer::FindTopDocuments(ExPol& policy, const std::string_view& raw_query, DocumentPredicate document_predicate) const {
    Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(policy, query, document_predicate);
    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (std::abs(lhs.relevance - rhs.relevance) < ALMOST_IRRELEVANT) {
            return lhs.rating > rhs.rating;
        }
        else {
            return lhs.relevance > rhs.relevance;
        }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate, typename ExPol>
std::vector<Document> SearchServer::FindAllDocuments(ExPol& policy, Query& query, DocumentPredicate document_predicate) const {

    ConcurrentMap <int, double> pre_document_to_relevance(100);
    std::for_each(policy, query.plus_words.begin(), query.plus_words.end(),
        [&document_predicate, &pre_document_to_relevance, this](const std::string_view& word) {
            if (word_to_document_freqs_.count(word) != 0) {
                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

                std::for_each(word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),
                    [&document_predicate, &pre_document_to_relevance, inverse_document_freq, this](const std::pair<int, double>& p) {
                        const auto& document_data = documents_.at(p.first);
                        if (document_predicate(p.first, document_data.status, document_data.rating)) {
                            pre_document_to_relevance[p.first].ref_to_value += p.second * inverse_document_freq;
                        }
                    });
            }});

    std::for_each(policy, query.minus_words.begin(), query.minus_words.end(),
        [this, &pre_document_to_relevance](const std::string_view& word) {
            if (word_to_document_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    pre_document_to_relevance.Erase(document_id);
                }
            }
        });
    std::map<int, double> document_to_relevance = pre_document_to_relevance.BuildOrdinaryMap();
    std::vector<Document> matched_documents;
    matched_documents.reserve(document_to_relevance.size());
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}