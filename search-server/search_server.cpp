#include "search_server.h"
#include "document.h"
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}
void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    if (document_id < 0) {
        throw std::invalid_argument("ID не должно быть отрицательным");
    }

    if (documents_.count(document_id) > 0) {
        throw std::invalid_argument("ƒокумент с таким ID уже добавлен");
    }
    std::vector<std::string> words = SplitIntoWordsNoStop(document);
    int w_size = words.size();
    const double inv_word_count = 1.0 / w_size;
    std::map<std::string, double> words_with_freqs;
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        double word_freq = count(words.begin(), words.end(), word) / w_size;
        words_with_freqs.emplace(word, word_freq);
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);
    document_to_word_freqs_.emplace(document_id, words_with_freqs);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string, double> empty_map;
    auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
    return it != document_ids_.end() ? document_to_word_freqs_.at(document_id) : empty_map;
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    Query query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

void SearchServer::RemoveDocument(int document_id) {
    documents_.erase(document_id);
    document_to_word_freqs_.erase(document_id);

    //чистим word_to_document_freqs
    //сначала чистим value
    for (auto& [key, value] : word_to_document_freqs_) {
        for (auto it = value.begin(); it != value.end(); ) {
            if (it->first == document_id) {
                it = value.erase(it);
            }
            else {
                ++it;
            }
        }
    }
    //если вдруг окажетс€, что слово было только в этом документе, то удалим его из word_to_document_freqs
    for (auto it = word_to_document_freqs_.begin(); it != word_to_document_freqs_.end(); ) {
        if (it->second.empty()) {
            it = word_to_document_freqs_.erase(it);
        }
        else {
            ++it;
        }
    }
    //в AddDocument есть проверка на повтор id, все id один раз, можно чистить не в цикле
    auto iter = find(document_ids_.begin(), document_ids_.end(), document_id);
    if (iter != document_ids_.end()) {
        document_ids_.erase(iter);
    }
}

bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Ќедопустимые символы в слове: " + word);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const {
    if (text.empty()) {
        throw std::invalid_argument("ќшибка в тексте запроса");
    }
    bool is_minus = false;
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
        throw std::invalid_argument("ќшибка в тексте запроса");
    }
    return QueryWord{ text, is_minus, IsStopWord(text) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const { //разбор запроса
    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            }
            else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}