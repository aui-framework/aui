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

//
// Created by nelonn on 9/4/25.
//

#pragma once

#include <AUI/Common/AUtf8.h>

template<typename OutT, typename InT>
constexpr const OutT* pointer_cast(const InT* ptr) {
    static_assert(sizeof(InT) == sizeof(OutT), "Size mismatch");
    static_assert(alignof(InT) == alignof(OutT), "Alignment mismatch");

    union Converter {
        const InT* from;
        const OutT* to;

        constexpr Converter(const InT* p) : from(p) {}
    };

    return Converter(ptr).to;
}

template<typename OutT, typename InT>
constexpr const OutT* pointer_cast(InT* ptr) {
    static_assert(sizeof(InT) == sizeof(OutT), "Size mismatch");
    static_assert(alignof(InT) == alignof(OutT), "Alignment mismatch");

    union Converter {
        InT* from;
        OutT* to;

        constexpr Converter(InT* p) : from(p) {}
    };

    return Converter(ptr).to;
}

class AByteBuffer;

class API_AUI_CORE AStringView: public std::string_view {
private:
    using super = std::string_view;

public:
    using bytes_type = super;

    using iterator = AUtf8ConstIterator;
    using const_iterator = AUtf8ConstIterator;
    using reverse_iterator = AUtf8ConstReverseIterator;
    using const_reverse_iterator = AUtf8ConstReverseIterator;

    using super::super;

    constexpr AStringView(const char8_t* utf8_str, size_t length) noexcept : super(pointer_cast<char>(utf8_str), length) {}

    constexpr AStringView(const char8_t* utf8_str) noexcept : super(pointer_cast<char>(utf8_str)) {}

    constexpr AStringView(std::string_view str) noexcept : super(str) {}

    bool startsWith(AStringView prefix) const noexcept {
        if (prefix.size() > size()) {
            return false;
        }
        return substr(0, prefix.size()) == prefix;
    }

    bool endsWith(AStringView suffix) const noexcept {
        if (suffix.size() > size()) {
            return false;
        }
        return substr(size() - suffix.size()) == suffix;
    }

    bool contains(char c) const noexcept;

    bool contains(AChar c) const noexcept;

    bool contains(AStringView str) const noexcept;

    /**
     * @brief Encodes the string into a null-terminated byte buffer using the specified encoding.
     * @sa bytes, toWideString
     */
    AByteBuffer encode(AStringEncoding encoding) const;

#if AUI_PLATFORM_WIN
    /**
     * @brief Encodes the string into a UTF-16 bytes stored in wchar_t for Windows API usage
     * @sa bytes, encode
     */
    std::wstring toWideString() const;
#endif

    /**
     * @brief Encodes the UTF-8 string into a UTF-16 string
     * @sa bytes, encode
     */
    std::u16string toUtf16() const;

    /**
     * @brief Encodes the UTF-8 string into a UTF-32 string
     * @sa bytes, encode
     */
    std::u32string toUtf32() const;

    std::string_view bytes() const noexcept {
        return *this;
    }

    iterator begin() const noexcept {
        return AUtf8ConstIterator(data(), data(), data() + size(), 0);
    }

    iterator end() const noexcept {
        return AUtf8ConstIterator(data(), data(), data() + size(), size());
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    reverse_iterator rbegin() const noexcept {
        return AUtf8ConstReverseIterator(end());
    }

    reverse_iterator rend() const noexcept {
        return AUtf8ConstReverseIterator(begin());
    }

    const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    const_reverse_iterator crend() const noexcept {
        return rend();
    }

    AChar first() const {
        if (empty()) {
            return AChar();
        }
        return *begin();
    }

    AChar last() const {
        if (empty()) {
            return AChar();
        }
        auto it = end();
        --it;
        return *it;
    }

};

template<>
struct std::hash<AStringView>
{
    size_t operator()(const AStringView& t) const noexcept
    {
        return std::hash<std::string_view>()(t);
    }
};

