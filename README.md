# Поисковый сервер с поддержкой многопоточности

### Ядро посковой системы, class SearchServer:
1. [search_server.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/search_server.h)
2. [search_server.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/search_server.cpp)

Добавлены многопоточные версии методов FindTopDocuments ([test](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/test_example.h)), FindAllDocuments, MatchDocument и RemoveDocument

Потокобезопасный class ConcurrentMap [concurrent_map.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/concurrent_map.h)
<!-- Найти тесты, написать тесты, добавить фреймворк, устранить дублирование removeDocument -->

#### Функционал разбиения результатов поиска на страницы:
 [paginator.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/paginator.h)
 
#### Обработка очереди запросов, class RequestQueue:
 1. [request_queue.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/request_queue.h)
 2. [request_queue.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/request_queue.cpp)
 
#### Многопоточная обработка запросов к поисковой системе
 1. [process_queries.h](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/process_queries.h)
 2. [process_queries.cpp](https://github.com/tatiana90st/cpp-search-server/blob/main/search-server/process_queries.cpp)
 
