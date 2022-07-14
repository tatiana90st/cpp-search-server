#include "document.h"
#include "search_server.h"
#include <iostream>
#include <stdexcept>
#include <tuple>

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка добавления документа " << document_id << ": " << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    std::cout << "Результаты поиска по запросу: " << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка поиска: " << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        std::cout << "Матчинг документов по запросу: " << query << std::endl;
        for (const int document_id : search_server) {
            std::tuple<std::vector<std::string_view>, DocumentStatus> result = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, std::get<0>(result), std::get<1>(result));
        }
    }
    catch (const std::exception& e) {
        std::cout << "Ошибка матчинга документов на запрос " << query << ": " << e.what() << std::endl;
    }
}

void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> to_be_removed;
    std::set<std::set<std::string_view>> words_in_docs;
    for (int id : search_server) {
        const std::map<std::string_view, double>& words_in_this_doc = search_server.GetWordFrequencies(id);
        std::set<std::string_view> words;
        for (const auto& [key, _] : words_in_this_doc) {
            words.insert(key);
        }
        auto result = words_in_docs.insert(words);
        if (!result.second) {
            to_be_removed.insert(id);
        }
    }
    for (int i : to_be_removed) {
        std::cout << "Found duplicate document id " << i << std::endl;
        search_server.RemoveDocument(i);
    }
}