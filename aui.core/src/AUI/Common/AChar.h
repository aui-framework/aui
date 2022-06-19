#pragma once


/**
 * @brief Represents a single 32-bit char.
 * @ingroup core
 */
class AChar {
private:
    char32_t mValue;

public:
    AChar(char c): mValue(c) {}

    [[nodiscard]]
    bool digit() const {
        return mValue >= '0' && mValue <= '9';
    }

    [[nodiscard]]
    bool alpha() const {
        return (mValue >= 'a' && mValue <= 'z') || (mValue >= 'A' && mValue <= 'Z');
    }

    [[nodiscard]]
    bool alnum() const {
        return alpha() || digit();
    }

    [[nodiscard]]
    char asAscii() const {
        return char(mValue);
    }

    operator char32_t() const {
        return mValue;
    }
};

static_assert(sizeof(AChar) == 4, "AChar should be exact 4 bytes");


