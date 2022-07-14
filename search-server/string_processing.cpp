#include<iostream>
#include<string>
#include<vector>
#include <string_view>
#include <set>
#include <map>

using namespace std::literals;

std::vector<std::string> SplitIntoWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}
//new
std::vector<std::string_view> SplitIntoWordsView(const std::string_view str1) {
    std::vector<std::string_view> result;
    std::string_view str = str1;
    str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    while (!str.empty()) {
        auto space = str.find(' ');
        //result.push_back(std::string(str.substr(0, space)));
        result.push_back(str.substr(0, space));
        str.remove_prefix(std::min(space, str.size()));
        str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    }
    return result;
}
//new
/*
std::set<std::string, std::less<>> VecIntoSet(const std::vector<std::string_view> words) {
    std::set<std::string, std::less<>> unique;
    for (const std::string_view word : words) {
        std::string w = static_cast<std::string>(word);
        unique.insert(w);
    }
    return unique;
}

*/


