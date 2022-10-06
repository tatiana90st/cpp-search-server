﻿#include "document.h"
#include <iostream>
#include <vector>

Document::Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating) {
}

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ document_id = " << document.id << ", relevance = " << document.relevance << ", rating = " << document.rating << " }";
    return out;
}

void PrintDocument(const Document& document) {
    std::cout << "{ "
        << "document_id = " << document.id << ", "
        << "relevance = " << document.relevance << ", "
        << "rating = " << document.rating << " }" << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status) {
    std::cout << "{ "
        << "document_id = " << document_id << ", "
        << "status = " << static_cast<int>(status) << ", "
        << "words =";
    for (const std::string_view& word : words) {
        std::cout << ' ' << word;
    }
    std::cout << "}" << std::endl;
}