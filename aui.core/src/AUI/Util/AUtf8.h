#pragma once


#include "AUI/Common/AString.h"
#include <type_traits>

template<typename Iterator>
class API_AUI_CORE AUtf8 {
static_assert(std::is_same_v<char, decltype(*std::declval<Iterator>())>, "iterator must deference to char");
private:
    Iterator mBegin;
    Iterator mEnd;

public:
    static constexpr auto INVALID_CHAR = L'�';

    class iterator {
    private:
        Iterator mValue;
        Iterator mEnd;

        [[nodiscard]]
        int currentOctetCount() const noexcept {
            char value = *mValue;
            if (!(value & 0b1000'0000)) {
                return 1;
            } else if (value >> 5 == 0b110) {
                return 2;
            } else if (value >> 4 == 0b1110) {
                return 3;
            } else if (value >> 3 == 0b11110) {
                return 4;
            }
            return 1;
        }

    public:
        iterator(Iterator value, Iterator end) noexcept : mValue(value), mEnd(end) {}

        [[nodiscard]]
        char32_t operator*() const noexcept {
            int octetCount = currentOctetCount();
            if (octetCount > mEnd - mValue) {
                return INVALID_CHAR;
            }
            switch (octetCount) {
                case 1: {
                    return *mValue;
                }
                case 2: {
                    char firstByte = *mValue;
                    char firstByte = *mValue;
                }
            }
        }

        AUtf8& operator++() noexcept {
            mValue += std::min(int(mEnd - mValue), currentOctetCount());
            return *this;
        }

        [[nodiscard]]
        bool operator==(const iterator& rhs) const noexcept {
            return mValue == rhs.mValue;
        }

        [[nodiscard]]
        bool operator!=(const iterator& rhs) const noexcept {
            return mValue != rhs.mValue;
        }
    };

    AUtf8(Iterator begin, Iterator end) : mBegin(begin), mEnd(end) {}

    [[nodiscard]]
    auto fromString(AStringView string) {
        return AUtf8(string.begin(), string.end());
    }
    iterator begin() const {
        return {mBegin};
    }
    iterator end() const {
        return {mEnd};
    }
};


