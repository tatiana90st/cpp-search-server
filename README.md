# Поисковый сервер с поддержкой многопоточности  
Обеспечивает поиск документов по ключевым словам и ранжирование результатов по TF-IDF, реализована функциональность минус-слов и стоп-слов. 

#### Ядро посковой системы, class SearchServer:
1. [search_server.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/search_server.h)
2. [search_server.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/search_server.cpp)  

Создание экземпляра класса SearchServer. В конструктор передаётся строка с стоп-словами, разделенными пробелами. Вместо строки можно передавать произвольный контейнер (с последовательным доступом к элементам с возможностью использования в for-range цикле)

С помощью метода AddDocument добавляются документы для поиска. В метод передаётся id документа, статус, рейтинг, и сам документ в формате строки.

Метод FindTopDocuments возвращает вектор документов, согласно соответствию переданным ключевым словам. Результаты отсортированы по TF-IDF.

Добавлены многопоточные версии методов FindTopDocuments ([test](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/test_example.h)), FindAllDocuments, MatchDocument и RemoveDocument

Потокобезопасный class ConcurrentMap [concurrent_map.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/concurrent_map.h)
<!-- Найти тесты, написать тесты, добавить фреймворк, устранить дублирование removeDocument -->

#### Функционал разбиения результатов поиска на страницы:
 [paginator.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/paginator.h)
 
#### Хранение истории запросов к поисковому серверу, class RequestQueue:
 1. [request_queue.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/request_queue.h)
 2. [request_queue.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/request_queue.cpp)  
 
Общее кол-во хранимых запросов не превышает заданного значения, новые запросы замещают самые старые запросы в очереди.
 
#### Многопоточная обработка запросов к поисковой системе (параллельное исполнение нескольких запросов)
 1. [process_queries.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/process_queries.h)
 2. [process_queries.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/process_queries.cpp)
 
