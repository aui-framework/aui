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
#include <AUI/Traits/values.h>
#include <fmt/format.h>

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

class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AStringVector;


/**
 * @brief Represents a UTF-8 string view.
 * @ingroup core
 * @details
 * AStringView stores a pointer and size of constant 8-bit integer sequence representing UTF-8 code units. Each Unicode
 * character (codepoint) is encoded using 1-4 consecutive code units, supporting the full Unicode standard.
 *
 * Unicode provides comprehensive support for international writing systems and symbols.
 *
 * AStringView points to constant data. For owning version of AStringView, see [AString].
 *
 * To work with raw bytes, use AStringView::bytes() function.
 */
class API_AUI_CORE AStringView: private std::string_view {
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

    bool operator==(std::string_view other) const noexcept {
        return bytes() == other;
    }

    bool operator!=(std::string_view other) const noexcept {
        return bytes() != other;
    }


    /**
     * @brief Raw `const char*` pointer to the string.
     * @return A pointer to the first character of the string.
     * @details
     * The returned pointer is not guaranteed to be valid C-style string, because we can't insert a null character at
     * the end.
     */
    [[nodiscard]]
    constexpr const char* data() const noexcept {
        return super::data();
    }

    /**
     * @brief Returns a substring `[pos, pos + count)`.
     * @param pos The starting position of the substring.
     * @param count The number of characters to include in the substring.
     * If the requested substring extends past the end of the string, i.e. the count is greater than `size() - pos`
     * (e.g. if `count == npos`), the returned substring is `[pos, size())`.
     *
     * Since AString encapsulates a UTF-8 encoded string, the returned substring is always valid UTF-8, hence, it
     * operates on top of UTF-8 code points. `pos` and `count` are interpreted as code points positions, not as byte.
     */
    [[nodiscard]]
    AStringView substr(size_type pos = 0, size_type count = npos) const noexcept;

    /**
     * @brief Returns the number of bytes in the UTF-8 encoded string
     * @sa length
     */
    size_type sizeBytes() const noexcept {
        return super::size();
    }

    /**
     * @brief Unchecked access to the UTF-8 character at the specified position.
     * @param i The position of the character to return.
     * @return The character at the specified position.
     */
    [[nodiscard]]
    AChar operator[](size_type i) const {
        if (empty()) {
            return AChar();
        }
        return *(begin() + i);
    }

    /**
     * @brief Returns the number of Unicode characters in the string
     * @sa sizeBytes
     */
    [[nodiscard]]
    size_type length() const noexcept;

    /**
     * @brief Encodes the string into a null-terminated byte buffer using the specified encoding.
     * @sa bytes, toUtf16, toUtf32
     */
    AByteBuffer encode(AStringEncoding encoding) const;

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


    /**
     * @brief Converts the string to boolean value.
     * @return If the string equals to "true", true returned, false otherwise.
     */
    bool toBool() const;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    AOptional<int32_t> toInt() const noexcept;

    /**
     * @brief Converts the string to long value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to long is not possible, nullopt is returned.
     */
    AOptional<int64_t> toLong() const noexcept;

    /**
     * @brief Converts the string to unsigned int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to unsigned int is not possible, exception is thrown.
     */
    AOptional<uint32_t> toUInt() const noexcept;

    /**
     * @brief Converts the string to unsigned long value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to unsigned long is not possible, exception is thrown.
     */
    AOptional<uint64_t> toULong() const noexcept;

    /**
     * @brief Converts the string to a float number.
     * @return The string converted to a float number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    AOptional<float> toFloat() const noexcept;

    /**
     * @brief Converts the string to a double number.
     * @return The string converted to a double number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    AOptional<double> toDouble() const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Returns std::nullopt if the conversion fails.
     * @sa toNumberOrException
     */
    AOptional<int> toNumber(aui::ranged_number<int, 2, 36> base) const noexcept;

    int32_t toIntOrException() const {
        return toInt().valueOrException(fmt::format("bad to number conversion: {}", bytes()).c_str());
    }

    int64_t toLongOrException() const {
        return toLong().valueOrException(fmt::format("bad to number conversion: {}", bytes()).c_str());
    }

    uint32_t toUIntOrException() const {
        return toUInt().valueOrException(fmt::format("bad to number conversion: {}", bytes()).c_str());
    }

    uint64_t toULongOrException() const {
        return toULong().valueOrException(fmt::format("bad to number conversion: {}", bytes()).c_str());
    }

    float toFloatOrException() const noexcept {
        return toDouble().valueOrException(fmt::format("bad float: {}", bytes()).c_str());
    }

    double toDoubleOrException() const noexcept {
        return toDouble().valueOrException(fmt::format("bad double: {}", bytes()).c_str());
    }

    int toNumberOrException(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const {
        return toNumber(base).valueOrException(fmt::format("bad to number conversion: {}", bytes()).c_str());
    }

    AStringVector split(AChar c) const;

    [[nodiscard]]
    AStringView trimLeft(AChar symbol = ' ') const;

    [[nodiscard]]
    AStringView trimRight(AChar symbol = ' ') const;

    [[nodiscard]]
    AStringView trim(AChar symbol = ' ') const
    {
        return trimLeft(symbol).trimRight(symbol);
    }

    [[nodiscard]]
    AString lowercase() const;

    [[nodiscard]]
    AString uppercase() const;

    AString removedAll(AChar c);

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;

};

template<>
struct std::hash<AStringView>
{
    size_t operator()(const AStringView& t) const noexcept
    {
        return std::hash<std::string_view>()(t.bytes());
    }
};

// gtest printer for AString
inline void PrintTo(AStringView s, std::ostream* stream) {
    *stream << s.bytes();
}

#if AUI_PLATFORM_WIN
namespace aui::win32 {
/*
 * On Windows, char16_t == wchar_t. WinAPI interfaces use wchar_t widely, so we have some handy functions to
 * convert AString(View) to wchar_t* and back.
 */

/**
 * @brief AString to const wchar_t*.
 * @ingroup core
 * @details
 * @exclusivefor{windows}
 */
API_AUI_CORE std::wstring toWchar(AStringView str);
}
#endif
