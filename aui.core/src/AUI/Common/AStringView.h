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
#include <fmt/core.h>

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
class AStringVector;

class API_AUI_CORE AStringView: public std::string_view {
private:
    using super = std::string_view;

public:
    using bytes_type = super;

    using iterator = AUtf8ConstIterator;
    using const_iterator = AUtf8ConstIterator;
    using reverse_iterator = AUtf8ConstReverseIterator;
    using const_reverse_iterator = AUtf8ConstReverseIterator;

    auto constexpr static NPOS = super::npos;

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;

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

    AStringView trimLeft(char symbol = ' ') const;
    AStringView trimRight(char symbol = ' ') const;
    AStringView trim(char symbol = ' ') const;

    size_type sizeBytes() const noexcept {
        return bytes().size();
    }

    AString uppercase() const;

    AString lowercase() const;

    AStringVector split(AChar c) const;

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
        return toInt().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    int64_t toLongOrException() const {
        return toLong().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    uint32_t toUIntOrException() const {
        return toUInt().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    uint64_t toULongOrException() const {
        return toULong().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    float toFloatOrException() const noexcept {
        return toDouble().valueOrException(fmt::format("bad float: {}", toStdString()).c_str());
    }

    double toDoubleOrException() const noexcept {
        return toDouble().valueOrException(fmt::format("bad double: {}", toStdString()).c_str());
    }

    int toNumberOrException(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const {
        return toNumber(base).valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
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

    std::string toStdString() const {
        return std::string{*this};
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
