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

#include <algorithm>
#include <string>
#include <iostream>
#include "AUI/Core.h"
#include "AUI/Traits/values.h"
#include <AUI/Common/ASet.h>
#include <optional>
#include <AUI/Common/AOptional.h>
#include <fmt/core.h>

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AByteBufferView;

/**
 * @brief Represents a Unicode character string.
 * @ingroup core
 * @details
 * AString stores a string of 16-bit chars, where each char corresponds to one UTF-16 code unit. Unicode characters with
 * code values above 65535 are stored using two consecutive chars.
 *
 * Unicode is an international standard that supports most of the writing systems in use today.
 */
class API_AUI_CORE AString: std::u16string
{
private:
    friend struct std::hash<AString>;
    using super = std::u16string;

public:

    using iterator = super::iterator;
    using value_type = super::value_type;
    using const_iterator = super::const_iterator;
    using reverse_iterator = super::reverse_iterator;
    using const_reverse_iterator = super::const_reverse_iterator;
    auto constexpr static NPOS = super::npos;


    AString(AString&& other) noexcept
            : std::u16string(static_cast<basic_string&&>(other))
    {
    }

    /**
     * @param other utf8 string
     */
    AString(const basic_string& other) noexcept
            : basic_string<char16_t>(other)
    {
    }

    /**
     * @param utf8 utf8 string
     */
    AString(const std::string& utf8) noexcept;

    AString(const AString& other) noexcept
            : super(other.c_str())
    {
    }

    AString(const basic_string& rhs, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(rhs, allocator)
    {
    }

    template <class Iterator>
    AString(Iterator first, Iterator last) noexcept : super(first, last) {}

    AString() noexcept
    {
    }

    AString(char16_t c) noexcept : super(&c, &c + 1)
    {

    }

    /**
     * @param utf8 utf8 string
     */
    AString(const char* utf8) noexcept;

    /**
     * @param utf8 utf8 string
     */
    AString(std::string_view utf8) noexcept;

    explicit AString(const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(allocator)
    {
    }

    AString(const basic_string& rhs, size_type offset, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(rhs, offset, allocator)
    {
    }

    AString(const basic_string& rhs, size_type offset, size_type count, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(rhs, offset, count, allocator)
    {
    }

    AString(const char16_t* cStyleString, size_type count) noexcept
            : basic_string<char16_t>(cStyleString, count)
    {
    }

    AString(const char16_t* cStyleString, size_type count, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(cStyleString, count, allocator)
    {
    }

    AString(const char16_t* cStyleString) noexcept
            : basic_string<char16_t>(cStyleString)
    {
    }

    AString(const char16_t* cStyleString, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(cStyleString, allocator)
    {
    }

    AString(size_type count, char16_t _Ch) noexcept
            : basic_string<char16_t>(count, _Ch)
    {
    }

    AString(size_type count, char16_t _Ch, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(count, _Ch, allocator)
    {
    }

    AString(basic_string&& rhs) noexcept
            : basic_string<char16_t>(std::move(rhs))
    {
    }

    AString(basic_string&& rhs, const std::allocator<char16_t>& allocator) noexcept
            : basic_string<char16_t>(std::move(rhs), allocator)
    {
    }

    AString(std::initializer_list<char16_t> _Ilist) noexcept
            : basic_string<char16_t>(_Ilist)
    {
    }

    ~AString() = default;


    void push_back(char16_t c) noexcept
    {
        super::push_back(c);
    }
    void pop_back() noexcept
    {
        super::pop_back();
    }

    AString uppercase() const;
    AString lowercase() const;

    bool startsWith(const AString& other) const noexcept
    {
        return rfind(other, 0) == 0;
    }
    bool startsWith(char16_t c) const noexcept
    {
        return rfind(c, 0) == 0;
    }
    bool endsWith(const AString& other) const noexcept
    {
        if (length() < other.length())
        {
            return false;
        }
        size_t offset = length() - other.length();
        return super::find(other, offset) == offset;
    }
    bool endsWith(char16_t c) const noexcept
    {
        size_t offset = length() - 1;
        return super::find(c, offset) == offset;
    }

    AStringVector split(char16_t c) const noexcept;

    size_type find(char c, size_type offset = 0) const noexcept
    {
        return super::find(c, offset);
    }
    size_type find(char16_t c, size_type offset = 0) const noexcept
    {
        return super::find(c, offset);
    }
    size_type find(const AString& str, size_type offset = 0) const noexcept
    {
        return super::find(str, offset);
    }
    size_type rfind(char c, size_type offset = NPOS) const noexcept
    {
        return super::rfind(c, offset);
    }
    size_type rfind(char16_t c, size_type offset = NPOS) const noexcept
    {
        return super::rfind(c, offset);
    }
    size_type rfind(const AString& str, size_type offset = NPOS) const noexcept
    {
        return super::rfind(str, offset);
    }
    size_type length() const noexcept
    {
        return super::length();
    }
    AString trimLeft(char16_t symbol = ' ') const noexcept;
    AString trimRight(char16_t symbol = ' ') const noexcept;

    AString trim(char16_t symbol = ' ') const noexcept
    {
        return trimRight(symbol).trimLeft(symbol);
    }

    void reserve(size_t s)
    {
        super::reserve(s);
    }
    void resize(size_t s)
    {
        super::resize(s);
    }

    AString restrictLength(size_t s, const AString& stringAtEnd = "...") const;

    char16_t* data() noexcept
    {
        return super::data();
    }

    const char16_t* data() const noexcept
    {
        return super::data();
    }
    AString& replaceAll(const AString& from, const AString& to);
    [[nodiscard]] AString replacedAll(const AString& from, const AString& to) const;
    [[nodiscard]] inline AString replacedAll(char16_t from, char16_t to) const noexcept {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c : *this) {
            if (c == from) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }
    [[nodiscard]] inline AString replacedAll(const ASet<char16_t>& from, char16_t to) const noexcept {
        AString copy;
        copy.reserve(length() + 10);
        for (auto c : *this) {
            if (from.contains(c)) {
                copy << to;
            } else {
                copy << c;
            }
        }
        return copy;
    }
    AString& replaceAll(char16_t from, char16_t to) noexcept;


    /**
     * @brief Inserts all values of the specified container to the end.
     * @tparam OtherContainer other container type.
     * @param c other container
     */
    template<typename OtherContainer>
    void insertAll(const OtherContainer& c) noexcept {
        super::insert(super::end(), c.begin(), c.end());
    }

    /**
     * @brief Converts the string to a float number.
     * @return The string converted to a float number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<float> toFloat() const noexcept;

    /**
     * @brief Converts the string to a double number.
     * @return The string converted to a double number.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<double> toDouble() const noexcept;

    /**
     * @brief Converts the string to a double number.
     * @return The string converted to a double number.
     *
     * If conversion to int is not possible, exception is thrown.
     */
    [[nodiscard]]
    double toDoubleOrException() const noexcept {
        return toDouble().valueOrException(fmt::format("bad double: {}", toStdString()).c_str());
    }

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<int> toInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, exception is thrown.
     */
    [[nodiscard]]
    int toIntOrException() const {
        return toInt().valueOrException(fmt::format("bad int: {}", toStdString()).c_str());
    }

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    AOptional<int64_t> toLongInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, nullopt is returned.
     */
    [[nodiscard]]
    int64_t toLongIntOrException() const {
        return toLongInt().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, exception is thrown.
     */
    [[nodiscard]]
    AOptional<unsigned> toUInt() const noexcept;

    /**
     * @brief Converts the string to int value.
     * @return The string converted to an integer value using base 10. If the string starts with 0x or 0X, the base 16
     * used.
     *
     * If conversion to int is not possible, exception is thrown.
     */
    [[nodiscard]]
    unsigned toUIntOrException() const {
        return toUInt().valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    /**
     * @brief Converts the string to boolean value.
     * @return If the string equals to "true", true returned, false otherwise.
     */
    [[nodiscard]]
    bool toBool() const noexcept {
        return *this == "true";
    }

    [[nodiscard]]
    bool contains(char16_t c) const noexcept
    {
        return find(c) != npos;
    }
    [[nodiscard]]
    bool contains(const AString& other) const noexcept
    {
        return find(other) != npos;
    }

    static AString fromLatin1(const AByteBuffer& buffer);
    static AString fromUtf8(const AByteBufferView& buffer);
    static AString fromUtf8(const char* buffer, size_t length);
    static AString fromLatin1(const char* buffer);

    static AString numberHex(int i) noexcept;

    template<typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
    static AString number(T i) noexcept {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            if (i)
                return "true";
            return "false";
        } else {
            auto v = std::to_string(i);
            if constexpr (std::is_floating_point_v<T>) {
                // remove trailing zeros
                v.erase(v.find_last_not_of('0') + 1, std::u16string::npos);
                v.erase(v.find_last_not_of('.') + 1, std::u16string::npos);
            }
            return v;
        }
    }

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;


    /**
     * @brief Returns the string converted to an int using base. Returns std::nullopt if the conversion fails.
     * @sa toNumberOrException
     */
    AOptional<int> toNumber(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Throws an exception if the conversion fails.
     * @sa toNumber
     */
    int toNumberOrException(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const {
        return toNumber(base).valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }


    /**
     * @return utf8-encoded std::string.
     */
    std::string toStdString() const noexcept;

    void resizeToNullTerminator();

    iterator erase(const_iterator begin, const_iterator end) noexcept
    {
        return super::erase(begin, end);
    }
    iterator erase(const_iterator begin) noexcept
    {
        return super::erase(begin);
    }

    AString& erase(size_type offset) noexcept
    {
        super::erase(offset);
        return *this;
    }
    AString& erase(size_type offset, size_type count) noexcept
    {
        super::erase(offset, count);
        return *this;
    }

    AByteBuffer toUtf8() const noexcept;

    void removeAt(unsigned at) noexcept
    {
        AUI_ASSERT(at <= length());
        erase(begin() + at);
    }
    AString excessSpacesRemoved() const noexcept;

    iterator insert(size_type at, char16_t c) noexcept
    {
        AUI_ASSERT(at <= length());
        return super::insert(begin() + at, 1, c);
    }
    iterator insert(size_type at, const AString& c) noexcept
    {
        AUI_ASSERT(at <= length());
        return super::insert(begin() + at, c.begin(), c.end());
    }

    template<typename Iterator>
    iterator insert(const_iterator at, Iterator begin, Iterator end) noexcept
    {
        AUI_ASSERT(std::distance(super::cbegin(), at) <= length());
        return super::insert(at, begin, end);
    }

    AString& operator<<(char c) noexcept
    {
        append(1, c);
        return *this;
    }
    AString& operator<<(char16_t c) noexcept
    {
        append(1, c);
        return *this;
    }

    inline ::AString& operator+=(const AStringView& str) noexcept
    {
        append(str);
        return *this;
    }
    inline ::AString& operator+=(const char* str) noexcept
    {
        *this += AString(str);
        return *this;
    }

    [[nodiscard]] bool empty() const noexcept {
        return super::empty();
    }
    [[nodiscard]] size_type size() const noexcept {
        return super::size();
    }
    char16_t operator[](size_type index) const
    {
        return super::at(index);
    }
    char16_t& operator[](size_type index)
    {
        return super::at(index);
    }
    bool operator<(const AString& other) const noexcept
    {
        return compare(other) < 0;
    }

    void clear() noexcept
    {
        super::clear();
    }

    char16_t& front() noexcept
    {
        return super::front();
    }
    char16_t& back() noexcept
    {
        return super::back();
    }
    const char16_t& front() const noexcept
    {
        return super::front();
    }
    const char16_t& back() const noexcept
    {
        return super::back();
    }
    char16_t& first() noexcept
    {
        return super::front();
    }
    char16_t& last() noexcept
    {
        return super::back();
    }
    const char16_t& first() const noexcept
    {
        return super::front();
    }
    const char16_t& last() const noexcept
    {
        return super::back();
    }

    [[nodiscard]]
    AOptional<char16_t> firstOpt() const noexcept
    {
        if (empty()) {
            return std::nullopt;
        }
        return super::front();
    }

    [[nodiscard]]
    AOptional<char16_t> lastOpt() const noexcept
    {
        if (empty()) {
            return std::nullopt;
        }
        return super::back();
    }

    const char16_t* c_str() const
    {
        return super::c_str();
    }

    iterator begin() noexcept
    {
        return super::begin();
    }
    iterator end() noexcept
    {
        return super::end();
    }

    const_iterator begin() const noexcept
    {
        return super::begin();
    }
    const_iterator end() const noexcept
    {
        return super::end();
    }

    reverse_iterator rbegin() noexcept
    {
        return super::rbegin();
    }
    reverse_iterator rend() noexcept
    {
        return super::rend();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return super::rbegin();
    }
    const_reverse_iterator rend() const noexcept
    {
        return super::rend();
    }

    AString& append(const AString& s) noexcept
    {
        super::append(s);
        return *this;
    }

    AString& append(size_t count, char16_t ch) noexcept
    {
        super::append(count, ch);
        return *this;
    }

    AString& operator=(const AString& value) noexcept
    {
        super::operator=(value);
        return *this;
    }

    AString& operator=(AString&& value) noexcept
    {
        super::operator=(std::move(value));
        return *this;
    }

    bool operator==(const AString& other) const noexcept
    {
        if (size() != other.size()) {
            return false;
        }
        return std::memcmp(data(), other.data(), sizeInBytes()) == 0;
    }
    bool operator==(const char16_t* other) const noexcept
    {
        auto it = begin();
        for (; it != end(); ++it, ++other) {
            if (*it != *other) {
                return false;
            }
            if (*other == '\0') {
                return false;
            }
        }
        return *other == '\0';
    }

    [[nodiscard]]
    size_t sizeInBytes() const noexcept {
        return size() * sizeof(super::value_type);
    }

    bool operator!=(const AString& other) const noexcept
    {
        return !operator==(other);
    }
    bool operator!=(const char16_t* other) const noexcept
    {
        return !operator==(other);
    }

    bool operator==(const char* other) const noexcept
    {
        return *this == AString(other);
    }

    bool operator!=(const char* other) const noexcept
    {
        return *this != AString(other);
    }

    template<typename... Args>
    inline AString format(Args&&... args) const;

    AString processEscapes() const;

    AString& removeAll(char16_t c) noexcept {
        erase(std::remove(begin(), end(), c), end());
        return *this;
    }

    [[nodiscard]]
    AString substr(std::size_t offset, std::size_t count = npos) const {
        return super::substr(offset, count);
    }

private:
    /**
     * @brief Converts the string to integer values. Used in AString::toInt, AString::toUInt, etc.
     */
    template<typename T>
    AOptional<T> toNumberImpl() const noexcept;
};

inline AString operator+(const AString& l, const AString& r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& l, char16_t r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& one, const char* other) noexcept
{
    return one + AString(other);
}

inline AString operator+(const char* other, const AString& one) noexcept {
    return AString(other) + one;
}

inline AString operator+(char lhs, const AString& cs) noexcept
{
    AString s(lhs);
    s += cs;
    return s;
}

inline AString operator""_as(const char* str, size_t len)
{
    return {str};
}

inline std::ostream& operator<<(std::ostream& o, const AString& s)
{
    o << s.toStdString();
    return o;
}

template<>
struct std::hash<AString>
{
    size_t operator()(const AString& t) const noexcept
    {
        return std::hash<std::u16string>()(t);
    }
};

#if AUI_PLATFORM_WIN
namespace aui::win32 {
    /*
     * On Windows, char16_t == wchar_t. WinAPI interfaces use wchar_t widely, so we have some handy functions to
     * convert AString to wchar_t* and back.
     */

    /**
     * @brief AString to const wchar_t*.
     * @ingroup core
     * @details
     * @exclusivefor{windows}
     */
    inline const wchar_t* toWchar(const AString& string) {
        // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
        return reinterpret_cast<const wchar_t *const>(string.data());
    }

    /**
     * @brief AString to const wchar_t*.
     * @ingroup core
     * @details
     * @exclusivefor{windows}
     */
    inline wchar_t* toWchar(AString& string) {
        // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
        return reinterpret_cast<wchar_t*>(string.data());
    }

    /**
     * @brief AString to wchar_t string view.
     * @ingroup core
     * @details
     * @exclusivefor{windows}
     */
    inline std::wstring_view toWcharView(const AString& string) {
        return {toWchar(string), string.length() };
    }

    /**
     * @brief wchar_t string view to AString.
     * @ingroup core
     * @details
     * @exclusivefor{windows}
     */
    inline AString fromWchar(std::wstring_view string) {
        // NOLINTNEXTLINE(*-pro-type-reinterpret-cast)
        return {reinterpret_cast<const char16_t *>(string.data()), string.size()};
    }
}
#endif

template <> struct fmt::detail::is_string<AString>: std::false_type {};

template <> struct fmt::formatter<AString>: fmt::formatter<std::string> {
    auto format(const AString& s, format_context& ctx) const {
        return fmt::formatter<std::string>::format(s.toStdString(), ctx);
    }
};


// gtest printer for AString
inline void PrintTo(const AString& s, std::ostream* stream) {
    *stream << s.toStdString();
}
