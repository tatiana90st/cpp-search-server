#include "search_server.h"
#include <map>
#include <string>
#include <set>

bool CompareMaps(const std::map<std::string, double>& map1, const std::map<std::string, double>& map2) {
	if (map1.size() != map2.size()) {
		return false;
	}

	for (auto it1 = map1.begin(); it1 != map1.end(); ++it1) {
		const std::string& key = it1->first;
		if (!map2.count(key)) {
			return false;
		}
	}
    //так как мапы одинакового размера, достатточно перебора элементов только одной из них
	return true;
}

void RemoveDuplicates(SearchServer& search_server) {
    std::set<int> to_be_removed;
    for (auto it = search_server.begin(); it != search_server.end(); ++it) {
        int id = *it;
        for (auto it2 = next(it); it2 != search_server.end(); ++it2) {
            int id2 = *it2;
            if (CompareMaps(search_server.GetWordFrequencies(id), search_server.GetWordFrequencies(id2))) {
                to_be_removed.insert(id2);
            }       
        }
    }
    for (int i : to_be_removed) {
        std::cout << "Found duplicate document id " << i << std::endl;
        search_server.RemoveDocument(i);
    }
}