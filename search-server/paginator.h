#pragma once
#include <iostream>

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
        auto it = itbegin;
        Iterator b, e = it;
        int p = 0;
        do {
            if (distance(it, itend) >= page_size) {
                b = it;
                advance(it, page_size);
                e = it;
                p = page_size;

            }
            else {
                b = it;
                e = itend;
                p = distance(b, e);
            }
            pages_.push_back(IteratorRange(b, e, p));
        } while (e != itend);

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
    vector<IteratorRange<Iterator>> pages_;
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

