#include "search_server.h"
#include "document.h"
#include "log_duration.h"
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include <string_view>

using namespace std::literals;

SearchServer::SearchServer(const std::string_view& stop_words_text)
    : SearchServer(SplitIntoWordsView(stop_words_text))
{
}

SearchServer::SearchServer(const std::string& stop_words)
    : SearchServer(SplitIntoWordsView(stop_words))
{
}

void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings) {
    if (document_id < 0) {
        throw std::invalid_argument("ID не должен быть отрицательным"s);
    }

    if (documents_.count(document_id) > 0) {
        throw std::invalid_argument("Документ с таким ID уже добавлен"s);
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status, std::string(document) });
    std::vector<std::string_view> words = SplitIntoWordsNoStop(documents_.at(document_id).text);
    int w_size = words.size();
    const double inv_word_count = 1.0 / w_size;
    std::map<std::string_view, double> words_with_freqs;
    for (const std::string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        double word_freq = count(words.begin(), words.end(), word) / w_size;
        words_with_freqs.emplace(word, word_freq);
    }

    document_ids_.push_back(document_id);
    document_to_word_freqs_.emplace(document_id, words_with_freqs);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query) const {
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string_view, double> empty_map;
    auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
    return it != document_ids_.end() ? document_to_word_freqs_.at(document_id) : empty_map;
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view& raw_query, int document_id) const {
    if (!documents_.count(document_id)) {
        throw std::invalid_argument("No docs with this id"s);
    }
    Query query = ParseQuery(raw_query);
    std::set<std::string_view> matched_words;
    std::vector<std::string_view> matched_words_v;
    for (const std::string_view& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (document_to_word_freqs_.at(document_id).count(word)) {
            matched_words.insert(word);
        }
    }
    for (const std::string_view& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (document_to_word_freqs_.at(document_id).count(word)) {
            return { matched_words_v, documents_.at(document_id).status };
        }
    }
    matched_words_v.reserve(matched_words.size());
    for (const std::string_view& word : matched_words) {
        matched_words_v.push_back(word);
    }
    return { matched_words_v, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
(std::execution::sequenced_policy, const std::string_view& raw_query, int document_id) const {
    return SearchServer::MatchDocument(raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
(std::execution::parallel_policy, const std::string_view& raw_query, int document_id) const {
    if (!documents_.count(document_id)) {
        throw std::invalid_argument("No docs with this id"s);
    }

    Query query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words1;
    for (const std::string_view& word : query.minus_words) {//if any_of
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (document_to_word_freqs_.at(document_id).count(word)) {
            return { matched_words1, documents_.at(document_id).status };
        }
    }
    std::set<std::string_view> matched_words;
    for (const std::string_view& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (document_to_word_freqs_.at(document_id).count(word)) {
            matched_words.insert(word);
        }
    }
    std::vector<std::string_view> matched_words_v;
    matched_words_v.reserve(matched_words.size());
    for (const std::string_view& word : matched_words) {
        matched_words_v.push_back(word);
    }
    return { matched_words_v, documents_.at(document_id).status };
}

void SearchServer::RemoveDocument(int document_id) {
    auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
    if (it == document_ids_.end()) {
        return;
    }
    document_ids_.erase(it);
    documents_.erase(document_id);
    for (const auto& [word, _] : document_to_word_freqs_.at(document_id)) {
        word_to_document_freqs_.at(static_cast<std::string>(word)).erase(document_id);
    }
    document_to_word_freqs_.erase(document_id);
}


void SearchServer::RemoveDocument(std::execution::sequenced_policy, int document_id) {
    SearchServer::RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(std::execution::parallel_policy, int document_id) {
    using namespace std;
    auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
    if (it == document_ids_.end()) {
        return;
    }
    document_ids_.erase(it);

    auto iter = documents_.find(document_id);
    documents_.erase(iter);

    auto it3 = document_to_word_freqs_.find(document_id);
    std::vector<std::string_view>words;
    words.reserve(it3->second.size());
    for (const auto& [word, _] : it3->second) {
        words.push_back(word);
    }
    document_to_word_freqs_.erase(it3);
    auto del = [document_id, this](string_view& s) {word_to_document_freqs_.at(static_cast<std::string>(s)).erase(document_id); };
    for_each(execution::par, words.begin(), words.end(), del);

}

bool SearchServer::IsValidWord(const std::string_view& word) {
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

bool SearchServer::IsStopWord(const std::string_view& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const {
    std::vector<std::string_view> words_not_checked = SplitIntoWordsView(text);
    if (any_of(words_not_checked.begin(), words_not_checked.end(), [](const std::string_view& word) { return !IsValidWord(static_cast<std::string>(word)); })) {
        throw std::invalid_argument("Недопустимые символы в слове");
    }
    std::vector<std::string_view> words(words_not_checked.size());
    auto it = copy_if(words_not_checked.begin(), words_not_checked.end(), words.begin(),
        [this](const std::string_view& word) {return !IsStopWord(word); });
    words.erase(it, words.end());
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Текст документа не должен быть пустым"s);
    }
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("Ошибка в тексте документа"s);
    }
    return QueryWord{ text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view& text) const { //ðàçáîð çàïðîñà
    Query query;
    std::vector<std::string_view> s;
    s = SplitIntoWordsView(text);
    for (const std::string_view& word : s) {
        QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.push_back(query_word.data);
            }
            else {
                query.plus_words.push_back(query_word.data);
            }
        }
    }
    std::sort(query.plus_words.begin(), query.plus_words.end());
    auto last = std::unique(query.plus_words.begin(), query.plus_words.end());
    query.plus_words.erase(last, query.plus_words.end());
    std::sort(query.minus_words.begin(), query.minus_words.end());
    auto last1 = std::unique(query.minus_words.begin(), query.minus_words.end());
    query.minus_words.erase(last1, query.minus_words.end());
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}