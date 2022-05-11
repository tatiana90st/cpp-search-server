#pragma once
#include "search_server.h"
#include <vector>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {

        ++current_time_;
        if (current_time_ > min_in_day_) {
            if (requests_.front().has_result == false) {
                --no_results_;
            }
            requests_.pop_front();
        }
        std::vector<Document>search_result = search_server_.FindTopDocuments(raw_query, document_predicate);
        QueryResult d_results;
        d_results.raw_query = raw_query;
        d_results.result = search_result;
        if (!search_result.empty()) {
            d_results.has_result = true;
        }
        else {
            ++no_results_;
        }
        requests_.push_back(d_results);

        return search_result;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::string raw_query;
        bool has_result = false;
        std::vector<Document> result;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    int current_time_ = 0;
    int no_results_ = 0;
};