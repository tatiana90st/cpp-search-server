#include "search_server.h"
#include "request_queue.h"
#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "read_strings.h"
#include "log_duration.h"
#include "remove_duplicates.h"
#include <iostream>

using  std::string_literals::operator ""s;

/* //Test LogDuration
int main() {
    setlocale(LC_ALL, "Russian");
    SearchServer search_server("and at in"s);

    AddDocument(search_server, 1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 2, "curly dog and fancy collars", DocumentStatus::ACTUAL, { 1, 2, 3 });
    AddDocument(search_server, 3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    AddDocument(search_server, 4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server, 5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
    {
        LogDuration x("Operation time"s, std::cout);
        MatchDocuments(search_server, "big collar"s);
    }
    {
        LOG_DURATION_STREAM("Operation time"s, std::cout);
        FindTopDocuments(search_server, "big collar"s);
    }

    return 0;
}*/

/* //Test Paginator
int main() {
    SearchServer search_server("and with"s);

    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);

    // ������� ��������� ��������� �� ���������
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        cout << *page << endl;
        cout << "Page break"s << endl;
    }
}*/

/* Test ReadInputFunctions
int main() {
    setlocale(LC_ALL, "Russian");

    SearchServer search_server("� � ��"s);

    AddDocument(search_server, 1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, -1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server, 3, "������� �� ����\x12��� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server, 4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    FindTopDocuments(search_server, "�������� -��"s);
    FindTopDocuments(search_server, "�������� --���"s);
    FindTopDocuments(search_server, "�������� -"s);

    MatchDocuments(search_server, "�������� ��"s);
    MatchDocuments(search_server, "������ -���"s);
    MatchDocuments(search_server, "������ --��"s);
    MatchDocuments(search_server, "�������� - �����"s);
}*/
int main() {
    SearchServer search_server("and with"s);

    AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // �������� ��������� 2, ����� �����
    AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ������� ������ � ����-������, ������� ����������
    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ������� ���������� ��������� 1
    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ���������� ����� �����, ���������� �� ��������
    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ��� � id 6, �������� �� ������ �������, ������� ����������
    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ���� �� ��� �����, �� �������� ����������
    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ����� �� ������ ����������, �� �������� ����������
    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    RemoveDuplicates(search_server);
    std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    /*
    std::cout << std::endl << "LogDuration"s << std::endl;
    setlocale(LC_ALL, "Russian");
    SearchServer search_server1("and at in"s);

    AddDocument(search_server1, 1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server1, 2, "curly dog and fancy collars", DocumentStatus::ACTUAL, { 1, 2, 3 });
    AddDocument(search_server1, 3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    AddDocument(search_server1, 4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server1, 5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
    {
        LogDuration x("Operation time"s, std::cout);
        MatchDocuments(search_server1, "big collar"s);
    }
    {
        LOG_DURATION_STREAM("Operation time"s, std::cout);
        FindTopDocuments(search_server1, "big collar"s);
    }

    std::cout << std::endl<<"Exceptions"s << std::endl;
    SearchServer search_server2("� � ��"s);

    AddDocument(search_server2, 1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server2, 1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server2, -1, "�������� �� � ������ �������"s, DocumentStatus::ACTUAL, { 1, 2 });
    AddDocument(search_server2, 3, "������� �� ����\x12��� �������"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    AddDocument(search_server2, 4, "������� �� ������� �������"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    FindTopDocuments(search_server2, "�������� -��"s);
    FindTopDocuments(search_server2, "�������� --���"s);
    FindTopDocuments(search_server2, "�������� -"s);

    MatchDocuments(search_server2, "�������� ��"s);
    MatchDocuments(search_server2, "������ -���"s);
    MatchDocuments(search_server2, "������ --��"s);
    MatchDocuments(search_server2, "�������� - �����"s);

    std::cout << std::endl << "Paginator"s << std::endl;

    SearchServer search_server3("and with"s);

    search_server3.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server3.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server3.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server3.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server3.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });

    const auto search_results = search_server3.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);

    // ������� ��������� ��������� �� ���������
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        std::cout << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }
    */
    return 0;

}