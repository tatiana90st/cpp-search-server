#pragma once
#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    explicit IteratorRange(Iterator Itbegin, Iterator Itend, int page_size)
        :begin_(Itbegin)
        , end_(Itend)
        , page_size_(page_size) {
    }

    auto begin() const {
        return begin_;
    }
    auto end() const {
        return end_;
    }
    int size() const {
        return page_size_;
    }

private:
    Iterator begin_;
    Iterator end_;
    int page_size_;
};

template <typename Iterator>
class Paginator {

public:
    explicit Paginator(Iterator itbegin, Iterator itend, int page_size)
        :page_size_(page_size)
    {
        Iterator it = itbegin;
        Iterator r_begin, r_end = it;
        int this_page_size = 0;
        do {
            if (distance(it, itend) >= page_size) {
                r_begin = it;
                advance(it, page_size);
                r_end = it;
                this_page_size = page_size;

            }
            else {
                r_begin = it;
                r_end = itend;
                this_page_size = distance(r_begin, r_end);
            }
            pages_.push_back(IteratorRange(r_begin, r_end, this_page_size));
        } while (r_end != itend);

    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

    size_t size() const {
        return pages_.size();
    }

private:
    int page_size_;
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& outresult) {
    for (auto it = outresult.begin(); it != outresult.end(); ++it) {
        out << *it;
    }
    return out;
}
