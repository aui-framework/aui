#include "AStringView.h"
#include <AUI/Common/AString.h>

int AStringView::find(AStringView str, std::size_t offset) const noexcept {
    assert(("str could not be empty", str.empty()));
    auto it = std::search(begin() + offset, end(), str.begin(), str.end());

    if (it == end()) return -1;
    return std::distance(begin(), it);
}

int AStringView::rfind(AStringView str, std::size_t offset) const noexcept {
    assert(("str could not be empty", str.empty()));
    auto it = std::search(rbegin() + offset, rend(), str.rbegin(), str.rend());

    if (it == rend()) return -1;
    return std::distance(begin(), it.base() + 1) + str.mLength;
}

AString AStringView::removedAll(wchar_t c) const {
    AString copy(*this);
    copy.removeAll(c);
    return copy;
}


template<typename T>
std::optional<T> AStringView::toNumberImpl() const noexcept {
    T value = 0;
    T prevValue = 0;
    bool negative = false;

    if constexpr (std::is_integral_v<T>) {
        if (startsWith("0x") || startsWith("0X")) {
            // hex
            for (auto c : substr(2)) {
                value *= 16;
                if (value < prevValue) { // overflow check
                    return std::nullopt;
                }
                prevValue = value;
                if (c >= '0' && c <= '9') {
                    value += c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    value += (c - 'a') + 10;
                } else if (c >= 'A' && c <= 'F') {
                    value += (c - 'A') + 10;
                } else {
                    return std::nullopt;
                }
            }
        } else {
            auto i = begin();
            if (*i == '-') {
                negative = true;
                ++i;
            }
            for (; i != end(); ++i) {
                value *= 10;
                if (value < prevValue) { // overflow check
                    return std::nullopt;
                }
                prevValue = value;
                auto c = *i;
                if (c >= '0' && c <= '9') {
                    value += c - '0';
                } else {
                    return std::nullopt;
                }
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        bool fractionalPart = false;
        double fractionalPower = 0.1;

        auto i = begin();
        if (*i == '-') {
            negative = true;
            ++i;
        }
        for (; i != end(); ++i) {
            auto c = *i;
            if (c >= '0' && c <= '9') {
                T digitValue = c - '0';
                if (fractionalPart) {
                    value += digitValue * fractionalPower;
                    fractionalPower *= 0.1;
                } else {
                    value *= 10;
                    value += digitValue;
                }
            } else if (c == '.') {
                if (fractionalPart) {
                    return std::nullopt;
                }
                fractionalPart = true;
            } else {
                return std::nullopt;
            }
        }
    }

    return negative ? -value : value;
}

std::optional<int> AStringView::toInt() const noexcept {
    return toNumberImpl<int>();
}

std::optional<unsigned> AStringView::toUInt() const noexcept {
    return toNumberImpl<unsigned>();
}

std::optional<double> AStringView::toDouble() const noexcept {
    return toNumberImpl<double>();
}

std::optional<float> AStringView::toFloat() const noexcept {
    return toNumberImpl<float>();
}

AString AStringView::restrictedLength(size_t s, AStringView stringAtEnd) const {
    AString s = *this;
    s.restrictLength()
    return *this;
}

AString AStringView::replacedAll(AChar from, AChar to) const noexcept {
    AString copy = *this;
    copy.replaceAll(from, to);
    return copy;
}

AString AStringView::replacedAll(const ASet<AChar>& from, AChar to) const noexcept {
    AString copy = *this;
    copy.replaceAll(from, to);
    return copy;
}
