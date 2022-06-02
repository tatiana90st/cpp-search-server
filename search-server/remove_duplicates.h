#pragma once
#include "search_server.h"

bool CompareMaps(const std::map<std::string, double>& map1, const std::map<std::string, double>& map2);

void RemoveDuplicates(SearchServer& search_server);