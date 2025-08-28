/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AStaticVector.h>

/**
 * @brief Represents a single 32-bit char.
 * @ingroup core
 */
class AChar {
private:
    char32_t mValue;

    static constexpr bool isValidUnicode(char32_t codePoint) {
        return codePoint <= 0x10FFFF &&
               (codePoint < 0xD800 || codePoint > 0xDFFF);
    }

public:
    constexpr static char32_t INVALID_CHAR = 0xFFFD;

    constexpr AChar(char c) : mValue(static_cast<char32_t>(static_cast<unsigned char>(c))) {}

    constexpr AChar(char8_t c): mValue(static_cast<char32_t>(c)) {}

    constexpr AChar(wchar_t c): mValue(static_cast<char32_t>(c)) {
        if constexpr (sizeof(wchar_t) == 2) {
            if (c >= 0xD800 && c <= 0xDFFF) {
                mValue = INVALID_CHAR;
            }
        } else {
            if (!isValidUnicode(mValue)) {
                mValue = INVALID_CHAR;
            }
        }
    }

    constexpr AChar(char16_t c): mValue(static_cast<char32_t>(c)) {
        if (c >= 0xD800 && c <= 0xDFFF) {
            mValue = INVALID_CHAR;
        }
    }

    constexpr AChar(char32_t c): mValue(isValidUnicode(c) ? c : INVALID_CHAR) {}

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
    bool isAscii() const {
        return mValue <= 0x7F;
    }

    [[nodiscard]]
    char asAscii() const {
        return isAscii() ? static_cast<char>(mValue) : ' ';
    }

    AStaticVector<char, 4> toUtf8() const noexcept;

    char32_t codepoint() const noexcept {
        return mValue;
    }

    operator char32_t() const noexcept {
        return mValue;
    }
};

static_assert(sizeof(AChar) == 4, "AChar should be exact 4 bytes");
