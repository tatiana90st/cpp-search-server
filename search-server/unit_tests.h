#pragma once
#include "test_framework.h"
#include "search_server.h"
#include "document.h"

#include <vector>
#include <string>
#include <string_view>

using namespace std;

void TestConstructor() {
    SearchServer search_server("  �  � ��   "s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    /*
    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s)) {
        PrintDocument(document);
    }
    */
    //ACTUAL by default
    ASSERT(search_server.FindTopDocuments("�������� ��������� ���"s).size() == 3);

    /*
    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
    */
    //BANNED
    ASSERT(search_server.FindTopDocuments("�������� ��������� ���"s, DocumentStatus::BANNED).size() == 1);
    /*
    cout << "Even ids:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("�������� ��������� ���"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    */
    //Predicate
    ASSERT(search_server.FindTopDocuments("�������� ��������� ���"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; }).size() == 2);
}

//���������� ����������. ����������� �������� ������ ���������� �� ���������� �������, ������� �������� ����� �� ���������
void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server("at with"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);

    }
}

//��������� �����-����. ���������, ���������� �����-����� ���������� �������, �� ������ ���������� � ���������� ������.
void TestExcludeMinusWordsFromSearch() {

    SearchServer server("at with"s);
    //������� ��������� ����������
    server.AddDocument(101, "harry potter and the philosopher's stone"s, DocumentStatus::ACTUAL, { 5, 2, 1 });
    server.AddDocument(102, "harry potter and the chamber of secrets"s, DocumentStatus::ACTUAL, { 5, 3, 2 });
    server.AddDocument(103, "harry potter and the prisoner of azkaban"s, DocumentStatus::ACTUAL, { 5, 4, 2 });
    server.AddDocument(104, "harry potter and the goblet of fire"s, DocumentStatus::ACTUAL, { 5, 5, 2 });
    //�������� ������ ��� ����� ����
    {
        const auto found_docs = server.FindTopDocuments("chamber"s);
        ASSERT(found_docs.size() == 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 102);
    }
    //� ������ � �������
    {
        const auto found_docs = server.FindTopDocuments("chamber -potter"s);
        ASSERT(found_docs.empty());
    }
}

//������� ����������. ��� �������� ��������� �� ���������� ������� ������ ���� ���������� ��� ����� 
// //�� ���������� �������, �������������� � ���������. 
//���� ���� ������������ ���� �� �� ������ �����-�����, ������ ������������ ������ ������ ����
void TestMatchDocument() {
    SearchServer server("at with"s);
    //������� ��������� ����������
    server.AddDocument(101, "harry potter and the philosopher's stone"s, DocumentStatus::ACTUAL, { 5, 2, 1 });
    server.AddDocument(102, "harry potter and the chamber of secrets"s, DocumentStatus::ACTUAL, { 5, 3, 2 });
    server.AddDocument(103, "harry potter and the prisoner of azkaban"s, DocumentStatus::ACTUAL, { 5, 4, 2 });
    server.AddDocument(104, "harry potter and the goblet of fire"s, DocumentStatus::ACTUAL, { 5, 5, 2 });
    //�������� ��� ������
    {
        vector<string_view> matched_words = get<0>(server.MatchDocument("harry potter and the order of phoenix"s, 104));
        ASSERT(matched_words.size() == 5);
    }
    //�������� � �������
    {
        vector<string_view> matched_words = get<0>(server.MatchDocument("-harry -potter and the order of phoenix"s, 104));
        ASSERT(matched_words.empty());
    }
}

//���������� ��������� ���������� �� �������������. 
//������������ ��� ������ ���������� ���������� ������ ���� ������������� � ������� �������� �������������
void TestSortMostRelevant() {
    SearchServer server("at with"s);
    //������� ��������� ����������
    server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    //����� ������
    const auto found_docs = server.FindTopDocuments("�������� ��������� ���"s);
    ASSERT(found_docs.size() == 3);

    const Document& doc0 = found_docs[0];
    const Document& doc1 = found_docs[1];
    ASSERT(doc0.relevance >= doc1.relevance);

}

//���������� ���������� ������������� ��������� ����������
//���������� �������� ����������. 
//������� ������������ ��������� ����� �������� ��������������� ������ ���������
void TestRelevanceAndRatingCalcCorrect() {
    SearchServer server("at with"s);
    //������� ��������� ����������
    server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    //����� ������
    const auto found_docs = server.FindTopDocuments("�������� ��������� ���"s);
    ASSERT(found_docs.size() == 3);
    const double IRRELEVANT_DIFFERENCE = 1e-4;
    const Document& doc0 = found_docs[0];
    ASSERT(abs(doc0.relevance - 0.6507) < IRRELEVANT_DIFFERENCE);
    ASSERT_EQUAL(doc0.rating, 5);
    const Document& doc1 = found_docs[1];
    ASSERT(abs(doc1.relevance - 0.2746) < IRRELEVANT_DIFFERENCE);
    ASSERT_EQUAL(doc1.rating, -1);
}

//����� ����������, ������� �������� ������
void TestSetStatus() {
    SearchServer server("at with"s);
    server.AddDocument(101, "harry potter and the philosopher's stone"s, DocumentStatus::ACTUAL, { 5, 2, 1 });
    server.AddDocument(102, "harry potter and the chamber of secrets"s, DocumentStatus::IRRELEVANT, { 5, 3, 2 });
    server.AddDocument(103, "harry potter and the prisoner of azkaban"s, DocumentStatus::ACTUAL, { 5, 4, 2 });
    server.AddDocument(104, "harry potter and the goblet of fire"s, DocumentStatus::BANNED, { 5, 5, 2 });
    //�� ��������� ���� � ACTUAL, ��������
    auto found_docs = server.FindTopDocuments("harry potter"s);
    ASSERT(found_docs.size() == 2);
    //�������� �� �������    
    found_docs = server.FindTopDocuments("harry potter"s, DocumentStatus::ACTUAL);
    ASSERT(found_docs.size() == 2);
    found_docs = server.FindTopDocuments("harry potter"s, DocumentStatus::BANNED);
    ASSERT(found_docs.size() == 1);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, 104);
}

//���������� ����������� ������ � �������������� ���������, ����������� �������������.
void TestDocPredicate() {
    SearchServer server("at with"s);
    server.AddDocument(101, "harry potter and the philosopher's stone"s, DocumentStatus::ACTUAL, { 5, 2, 1 });
    server.AddDocument(102, "harry potter and the chamber of secrets"s, DocumentStatus::IRRELEVANT, { 5, -10, 2 });
    server.AddDocument(103, "harry potter and the prisoner of azkaban"s, DocumentStatus::ACTUAL, { 5, 4, 2 });
    server.AddDocument(104, "harry potter and the goblet of fire"s, DocumentStatus::BANNED, { 5, 5, 2 });
    auto found_docs = server.FindTopDocuments("harry potter"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
    ASSERT(found_docs.size() == 2);
    found_docs = server.FindTopDocuments("harry potter"s, [](int document_id, DocumentStatus status, int rating) { return rating < 0; });
    ASSERT_EQUAL(found_docs.size(), 1);
}

// ���� ���������, ��� ��������� ������� ��������� ����-����� ��� ���������� ����������
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server("at with"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

void TestExceptions() {

    {
        SearchServer search_server("� � ��"s);
        ASSERT_DOESNT_THROW(search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 }));
    }

    {
        SearchServer search_server("� � ��"s);
        search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        ASSERT_THROWS(search_server.AddDocument(1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 }), invalid_argument);
        ASSERT_THROWS(search_server.AddDocument(-1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 }), invalid_argument);
        ASSERT_THROWS(search_server.AddDocument(3, "������� �� ����\x12��� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 }), invalid_argument)
    }

    {
        SearchServer search_server("� � ��"s);
        search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
        ASSERT_THROWS(search_server.FindTopDocuments("�������� --���"s), invalid_argument);
        ASSERT_THROWS(search_server.FindTopDocuments("�������� -"s), invalid_argument);
        ASSERT_DOESNT_THROW(search_server.FindTopDocuments("�������� -��"s));

    }
}

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer() {
    TestRunner tr;
    RUN_TEST(tr, TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(tr, TestAddDocument);
    RUN_TEST(tr, TestExcludeMinusWordsFromSearch);
    RUN_TEST(tr, TestMatchDocument);
    RUN_TEST(tr, TestSortMostRelevant);
    RUN_TEST(tr, TestRelevanceAndRatingCalcCorrect);
    RUN_TEST(tr, TestSetStatus);
    RUN_TEST(tr, TestDocPredicate);
    RUN_TEST(tr, TestConstructor);
    RUN_TEST(tr, TestExceptions);
    cout << "End of unit testing"s << endl << endl;
}