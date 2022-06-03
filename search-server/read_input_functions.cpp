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
        std::cout << "������ ���������� ��������� " << document_id << ": " << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    std::cout << "���������� ������ �� �������: " << raw_query << std::endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const std::exception& e) {
        std::cout << "������ ������: " << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        std::cout << "������� ���������� �� �������: " << query << std::endl;
        for (const int document_id : search_server){
            std::tuple<std::vector<std::string>, DocumentStatus> result = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, std::get<0>(result), std::get<1>(result));
        }
    }
    catch (const std::exception& e) {
        std::cout << "������ �������� ���������� �� ������ " << query << ": " << e.what() << std::endl;
    }
}