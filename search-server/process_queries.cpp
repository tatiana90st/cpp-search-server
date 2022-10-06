#include "document.h"
#include "search_server.h"
#include <vector>
#include <algorithm>
#include <execution>
#include <iterator>
#include <list>


using namespace std;

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries) {
    vector<vector<Document>> search_results(queries.size());
    transform(execution::par,
        queries.begin(), queries.end(),
        search_results.begin(),
        [&search_server](const string& query) {
            return search_server.FindTopDocuments(query);
        }
    );
    return search_results;
}

list <Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    vector<vector<Document>> search_results(queries.size());
    transform(execution::par,
        queries.begin(), queries.end(),
        search_results.begin(),
        [&search_server](const string& query) {
            return search_server.FindTopDocuments(query);
        }
    );
    list <Document> result;
    for (auto& q : search_results) {
        copy(make_move_iterator(q.begin()), make_move_iterator(q.end()), back_inserter(result));
    };
    return result;
}