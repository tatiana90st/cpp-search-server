#include<iostream>
#include<string>
#include<vector>
#include <string_view>

using namespace std;

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

std::vector<std::string_view> SplitIntoWordsView(std::string_view str) {
    std::vector<std::string_view> result;
    str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    while (!str.empty()) {
        auto space = str.find(' ');
        result.push_back(str.substr(0, space));
        str.remove_prefix(std::min(space, str.size()));
        str.remove_prefix(std::min(str.find_first_not_of(" "), str.size()));
    }
    return result;
}