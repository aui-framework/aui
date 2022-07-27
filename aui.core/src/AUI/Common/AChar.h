#pragma once

#include <locale>

/**
 * @brief Represents a single 16-bit char.
 * @ingroup core
 */
class AChar {
private:
    wchar_t mValue;

public:
    AChar(wchar_t c): mValue(c) {}

    [[nodiscard]]
    bool whitespace() const noexcept {
        switch (mValue) {
            case ' ':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
            case '\v':
                return true;
            default: return false;
        }
    }

    [[nodiscard]]
    bool digit() const noexcept {
        return mValue >= '0' && mValue <= '9';
    }

    [[nodiscard]]
    bool alpha() const noexcept {
        return (mValue >= 'a' && mValue <= 'z') || (mValue >= 'A' && mValue <= 'Z');
    }

    [[nodiscard]]
    bool alnum() const noexcept {
        return alpha() || digit();
    }

    [[nodiscard]]
    char asAscii() const noexcept {
        return char(mValue);
    }

    operator wchar_t () const noexcept {
        return mValue;
    }

    bool operator==(AChar rhs) const noexcept {
        return mValue == rhs.mValue;
    }

    bool operator!=(AChar rhs) const noexcept {
        return mValue != rhs.mValue;
    }

    bool operator<(AChar rhs) const noexcept {
        return mValue < rhs.mValue;
    }

    bool operator<=(AChar rhs) const noexcept {
        return mValue <= rhs.mValue;
    }

    bool operator>(AChar rhs) const noexcept {
        return mValue > rhs.mValue;
    }

    bool operator>=(AChar rhs) const noexcept {
        return mValue >= rhs.mValue;
    }
};

static_assert(sizeof(AChar) == 2, "AChar should be exact 2 bytes");
