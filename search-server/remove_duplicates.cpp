#include "search_server.h"
#include <set>
#include <map>
#include <string>

void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> to_be_removed;
    std::set<std::set<std::string>> words_in_docs;
    for (int id : search_server) {
        const std::map<std::string, double>& words_in_this_doc = search_server.GetWordFrequencies(id);
        std::set<std::string> words;
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