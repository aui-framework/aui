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

#include <AUI/Common/detail/util.hpp>
#include <AUI/Common/AUtf8.hpp>
#include <AUI/Traits/values.h>
#include <fmt/format.h>

class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AStringVector;


/**
 * @brief Represents a string view.
 * @ingroup core
 * @details
 * AStringView stores a pointer and size of constant byte sequence.
 *
 * AStringView points to constant data. For owning version of AStringView, see [AString].
 *
 * To work with raw bytes, use AStringView::bytes() function.
 */
class API_AUI_CORE AStringView: public std::string_view {
private:
    using super = std::string_view;

public:
    using bytes_type = super;

    auto constexpr static NPOS = super::npos;

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;

    using super::super;

    constexpr AStringView(const char8_t* utf8_str, size_t length) noexcept : super(pointer_cast<char>(utf8_str), length) {}

    constexpr AStringView(const char8_t* utf8_str) noexcept : super(pointer_cast<char>(utf8_str)) {}

    constexpr AStringView(std::string_view str) noexcept : super(str) {}

    explicit AStringView(const std::string& str) noexcept : super(str) {}

    constexpr bool startsWith(char prefix) const noexcept {
        if (empty()) return false;
        return at(0) == prefix;
    }

    constexpr bool startsWith(AStringView prefix) const noexcept {
        if (prefix.size() > size()) {
            return false;
        }
        return bytes().substr(0, prefix.size()) == prefix.bytes(); // NOLINT(*-use-starts-ends-with)
    }

    constexpr bool endsWith(char prefix) const noexcept {
        if (empty()) return false;
        return at(sizeBytes() - 1) == prefix;
    }

    constexpr bool endsWith(AStringView suffix) const noexcept {
        if (suffix.size() > size()) {
            return false;
        }
        return bytes().substr(size() - suffix.size()) == suffix.bytes();
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return super::empty();
    }

    bool contains(char c) const noexcept;
    bool contains(AStringView str) const noexcept;

    constexpr bool operator==(AStringView other) const noexcept {
        return bytes() == other.bytes();
    }

    constexpr bool operator!=(AStringView other) const noexcept {
        return bytes() != other.bytes();
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
    constexpr AStringView substr(size_type pos = 0, size_type count = npos) const noexcept {
        return AStringView(super::substr(pos, count));
    }

    /**
     * @brief Returns the number of bytes in the UTF-8 encoded string
     * @sa length
     */
    size_type sizeBytes() const noexcept {
        return super::size();
    }

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

    constexpr std::string_view bytes() const noexcept {
        return *this;
    }

    constexpr AStringView trimLeft(AChar symbol = ' ') const {
        for (auto i = begin(); i != end(); ++i) {
            if (*i != symbol) {
                return AStringView(&*i, static_cast<size_t>(end() - i));
            }
        }
        return {};
    }

    constexpr AStringView trimRight(AChar symbol = ' ') const {
        for (auto i = rbegin(); i != rend(); ++i) {
            if (*i != symbol) {
                return AStringView(data(), static_cast<size_t>(i.base() - begin()));
            }
        }
        return {};
    }

    constexpr AStringView trim(AChar symbol = ' ') const {
        return trimLeft(symbol).trimRight(symbol);
    }

    AString uppercase() const;

    AString lowercase() const;

    AString replacedAll(AStringView from, AStringView to) const;

    AString replacedAll(AChar from, AChar to) const;

    AString removedAll(AStringView seq) const;

    AString removedAll(AChar c) const;

    AStringVector split(AStringView separator) const;

    AStringVector split(AChar separator) const;

    constexpr AChar first() const {
        if (empty()) {
            return AChar();
        }
        return *begin();
    }

    constexpr AChar last() const {
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

    std::string toStdString() const {
        return std::string(bytes());
    }

    AUtf8View utf8() const noexcept {
        return AUtf8View(bytes());
    }

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

inline std::ostream& operator<<(std::ostream& o, const AStringView& s)
{
    o << s.bytes();
    return o;
}


#if defined(FMT_VERSION) && (FMT_VERSION < 100000)
template <> struct fmt::detail::is_string<AStringView>: std::false_type {};
#endif

template <> struct fmt::formatter<AStringView>: fmt::formatter<std::string_view> {
    auto format(const AStringView& s, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(s.bytes(), ctx);
    }
};

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
