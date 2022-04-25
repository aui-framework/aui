#pragma once

#include <ostream>

template<typename Iterator>
struct AContainerPrinter {
    Iterator mBegin;
    Iterator mEnd;

    AContainerPrinter(Iterator mBegin, Iterator mEnd) : mBegin(mBegin), mEnd(mEnd) {}

    template<typename Container>
    AContainerPrinter(const Container& c): AContainerPrinter(c.begin(), c.end()) {}
};

template<typename Container>
AContainerPrinter(const Container& c) -> AContainerPrinter<decltype(c.begin())>;

template<typename Iterator>
inline std::ostream& operator<<(std::ostream& o, const AContainerPrinter<Iterator>& printer) noexcept {
    o << '{';
    for (auto it = printer.mBegin; it != printer.mEnd; ++it) {
        if (it != printer.mBegin) {
            o << ", ";
        }
        o << *it;
    }
    o << '}';
    return o;
}