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
#include <string_view>
#include <iostream>
#include <AUI/Core.h>
#include <AUI/Traits/values.h>
#include <AUI/Common/ASet.h>
#include <AUI/Common/AChar.h>
#include <optional>
#include <span>
#include <AUI/Common/AOptional.h>
#include <fmt/core.h>

namespace aui::detail {

char32_t decodeUtf8At(const char* data, size_t& bytePos, size_t maxSize) noexcept;

size_t getPrevCharStart(const char* data, size_t pos) noexcept;

std::optional<size_t> findUnicodePos(std::string_view utf8_str, size_t unicode_index);

}

/**
 * @brief UTF-8 forward iterator for AString
 */
class AUtf8ConstIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const char32_t*;
    using reference = char32_t;

private:
    const char* data_;
    const char* begin_;
    const char* end_;
    size_t byte_pos_;

public:
    constexpr AUtf8ConstIterator() noexcept
        : data_(nullptr), begin_(nullptr), end_(nullptr),
          byte_pos_(0) {}

    constexpr AUtf8ConstIterator(const char* data, const char* begin, const char* end, size_t pos) noexcept
        : data_(data), begin_(begin), end_(end),
          byte_pos_(pos) {}

    AChar operator*() const noexcept {
        size_t temp_pos = byte_pos_;
        return aui::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
    }

    AUtf8ConstIterator& operator++() noexcept {
        if (byte_pos_ < static_cast<size_t>(end_ - begin_)) {
            size_t temp_pos = byte_pos_;
            aui::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
            byte_pos_ = temp_pos;
        }
        return *this;
    }

    AUtf8ConstIterator operator++(int) noexcept {
        AUtf8ConstIterator temp = *this;
        ++(*this);
        return temp;
    }

    AUtf8ConstIterator& operator--() noexcept {
        if (byte_pos_ > 0) {
            byte_pos_ = aui::detail::getPrevCharStart(data_, byte_pos_);
        }
        return *this;
    }

    AUtf8ConstIterator operator--(int) noexcept {
        AUtf8ConstIterator temp = *this;
        --(*this);
        return temp;
    }

    AUtf8ConstIterator& operator+=(int n) noexcept {
        if (n > 0) {
            ++(*this);
        } else if (n < 0) {
            --(*this);
        }
        return *this;
    }

    AUtf8ConstIterator& operator-=(int n) noexcept {
        return *this += (-n);
    }

    AUtf8ConstIterator operator+(int n) const noexcept {
        AUtf8ConstIterator result = *this;
        result += n;
        return result;
    }

    AUtf8ConstIterator operator-(int n) const noexcept {
        AUtf8ConstIterator result = *this;
        result -= n;
        return result;
    }

    bool operator==(const AUtf8ConstIterator& other) const noexcept {
        return data_ == other.data_ && byte_pos_ == other.byte_pos_;
    }

    bool operator!=(const AUtf8ConstIterator& other) const noexcept {
        return !(*this == other);
    }

    size_t getBytePos() const noexcept {
        return byte_pos_;
    }

    AUtf8ConstIterator& operator=(const std::string::iterator& it) noexcept {
        if (begin_ != nullptr) {
            byte_pos_ = it.base() - begin_;
        }
        return *this;
    }

    AUtf8ConstIterator& operator=(const std::string::const_iterator& it) noexcept {
        if (begin_ != nullptr) {
            byte_pos_ = it.base() - begin_;
        }
        return *this;
    }
};

/**
 * @brief UTF-8 reverse iterator for AString
 */
class AUtf8ConstReverseIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const char32_t*;
    using reference = char32_t;

private:
    AUtf8ConstIterator base_iterator_;

public:
    explicit AUtf8ConstReverseIterator() noexcept = default;

    explicit AUtf8ConstReverseIterator(AUtf8ConstIterator it) noexcept
        : base_iterator_(it) {
        --base_iterator_;
    }

    AUtf8ConstIterator base() const noexcept {
        AUtf8ConstIterator temp = base_iterator_;
        ++temp;
        return temp;
    }

    char32_t operator*() const noexcept {
        return *base_iterator_;
    }

    AUtf8ConstReverseIterator& operator++() noexcept {
        --base_iterator_;
        return *this;
    }

    AUtf8ConstReverseIterator operator++(int) noexcept {
        AUtf8ConstReverseIterator temp = *this;
        ++(*this);
        return temp;
    }

    AUtf8ConstReverseIterator& operator--() noexcept {
        ++base_iterator_;
        return *this;
    }

    AUtf8ConstReverseIterator operator--(int) noexcept {
        AUtf8ConstReverseIterator temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ == other.base_iterator_;
    }

    bool operator!=(const AUtf8ConstReverseIterator& other) const noexcept {
        return !(*this == other);
    }

    AUtf8ConstReverseIterator& operator=(std::string::reverse_iterator it) noexcept {
        base_iterator_ = it.base();
        --base_iterator_;
        return *this;
    }

    AUtf8ConstReverseIterator& operator=(std::string::const_reverse_iterator it) noexcept {
        base_iterator_ = it.base();
        --base_iterator_;
        return *this;
    }
};


class AUtf8MutableIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const char32_t*;
    using reference = char32_t;

private:
    AString* string_;
    size_t byte_pos_;

    /**
     * @brief Get the byte length of UTF-8 character at current position
     */
    size_t getCurrentCharByteLength() const noexcept;

    /**
     * @brief Get the byte length needed to encode a char32_t in UTF-8
     */
    static size_t getEncodedByteLength(char32_t codepoint) noexcept;

    /**
     * @brief Encode a char32_t to UTF-8 bytes
     */
    static size_t encodeUtf8(char32_t codepoint, char* buffer) noexcept;

public:
    AUtf8MutableIterator() noexcept;
    AUtf8MutableIterator(AString* str, size_t pos) noexcept;

    /**
     * @brief Dereference operator - returns current character
     */
    AChar operator*() const noexcept;

    /**
     * @brief Assignment operator - replaces current character
     * @param c The character to assign at current position
     * @return Reference to this iterator
     */
    AUtf8MutableIterator& operator=(AChar c);

    /**
     * @brief Pre-increment operator
     */
    AUtf8MutableIterator& operator++() noexcept;

    /**
     * @brief Post-increment operator
     */
    AUtf8MutableIterator operator++(int) noexcept;

    /**
     * @brief Pre-decrement operator
     */
    AUtf8MutableIterator& operator--() noexcept;

    /**
     * @brief Post-decrement operator
     */
    AUtf8MutableIterator operator--(int) noexcept;

    /**
     * @brief Advance this iterator by n characters forward (in-place)
     * @param n Number of characters to advance (can be negative for backward movement)
     * @return Reference to this iterator
     */
    AUtf8MutableIterator& operator+=(int n) noexcept;

    AUtf8MutableIterator& operator-=(int n) noexcept {
        return *this += (-n);
    }

    AUtf8MutableIterator operator+(int n) const noexcept {
        AUtf8MutableIterator result = *this;
        result += n;
        return result;
    }

    AUtf8MutableIterator operator-(int n) const noexcept {
        AUtf8MutableIterator result = *this;
        result -= n;
        return result;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const AUtf8MutableIterator& other) const noexcept;

    /**
     * @brief Inequality comparison
     */
    bool operator!=(const AUtf8MutableIterator& other) const noexcept;

    /**
     * @brief Get current byte position
     */
    size_t getBytePos() const noexcept;

    /**
     * @brief Get pointer to the string
     */
    AString* getString() const noexcept;

    /**
     * @brief Convert to const iterator
     */
    operator AUtf8ConstIterator() const noexcept;
};

class AStringView: public std::string_view {
private:
    using super = std::string_view;

public:
    using bytes_type = super;

    using iterator = AUtf8ConstIterator;
    using const_iterator = AUtf8ConstIterator;
    using reverse_iterator = AUtf8ConstReverseIterator;
    using const_reverse_iterator = AUtf8ConstReverseIterator;

    using super::super;

    constexpr AStringView(const char8_t* utf8_str, size_t length) noexcept : super((const char*) utf8_str, length) {}

    constexpr AStringView(const char8_t* utf8_str) noexcept : super((const char*) utf8_str) {}

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

class API_AUI_CORE AStringVector;
class API_AUI_CORE AByteBuffer;
class API_AUI_CORE AByteBufferView;

enum class AStringEncoding : uint8_t {
    UTF8 = 0,
    UTF16 = 1,
    UTF32 = 2,
    LATIN1 = 3,
};

/**
 * @brief Represents a Unicode character string.
 * @ingroup core
 * @details
 * AString stores a string of 16-bit chars, where each char corresponds to one UTF-16 code unit. Unicode characters with
 * code values above 65535 are stored using two consecutive chars.
 *
 * Unicode is an international standard that supports most of the writing systems in use today.
 */
class API_AUI_CORE AString: public std::string
{
private:
    friend struct std::hash<AString>;
    using super = std::string;

public:

    using value_type = super::value_type;
    using bytes_type = super;

    using iterator = AUtf8MutableIterator;
    using const_iterator = AUtf8ConstIterator;
    using reverse_iterator = AUtf8ConstReverseIterator;
    using const_reverse_iterator = AUtf8ConstReverseIterator;

    auto constexpr static NPOS = super::npos;

    static AString numberHex(int i);

    template<typename T, std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
    static AString number(T i) noexcept {
        if constexpr (std::is_same_v<bool, std::decay_t<T>>) {
            if (i) return "true";
            return "false";
        } else {
            auto v = std::to_string(i);
            if constexpr (std::is_floating_point_v<T>) {
                // remove trailing zeros
                v.erase(v.find_last_not_of('0') + 1, std::string::npos);
                v.erase(v.find_last_not_of('.') + 1, std::string::npos);
            }
            return v;
        }
    }

    static constexpr auto TO_NUMBER_BASE_BIN = 2;
    static constexpr auto TO_NUMBER_BASE_OCT = 8;
    static constexpr auto TO_NUMBER_BASE_DEC = 10;
    static constexpr auto TO_NUMBER_BASE_HEX = 16;

    template<class T>
    constexpr static size_type strLength(const T* str) noexcept {
        if (str == nullptr) {
            return 0;
        }

        size_type length = 0;
        while (str[length] != T()) {
            ++length;
        }
        return length;
    }

    using super::super;

    AString(const AString& other) : super(other) {}

    AString(AString&& other) noexcept : super(std::move(other)) {}

    AString(std::span<const std::byte> bytes, AStringEncoding encoding);

    AString(const char* utf8_bytes, size_type length);

    AString(const char* utf8_bytes) : AString(utf8_bytes, strLength(utf8_bytes)) {}

    AString(const char8_t* utf8_bytes, size_type length) : AString((const char*) utf8_bytes, length) {}

    AString(const char8_t* utf8_bytes) : AString(utf8_bytes, strLength(utf8_bytes)) {}

    AString(std::u8string_view utf8_string) : AString(utf8_string.data(), utf8_string.size()) {}

    AString(const char16_t* utf16_bytes, size_type length);

    AString(const char16_t* utf16_bytes) : AString(utf16_bytes, strLength(utf16_bytes)) {}

    AString(std::u16string_view utf16_string) : AString(utf16_string.data(), utf16_string.size()) {}

    AString(const char32_t* utf32_bytes, size_type length);

    AString(const char32_t* utf32_bytes) : AString(utf32_bytes, strLength(utf32_bytes)) {}

    AString(std::u32string_view utf32_string) : AString(utf32_string.data(), utf32_string.size()) {}

    AString(AStringView view) : super(static_cast<std::string_view>(view)) {}

    AString(std::string_view view) : super(view) {}

    constexpr AString(const super& other) : super(other) {}

    constexpr AString(super&& other) : super(std::move(other)) {}

    AString(AChar c);

    AString(size_type n, AChar c);

    AString(std::initializer_list<AChar> il) {
        reserve(il.size());
        for (AChar c : il) {
            push_back(c);
        }
    }

    ~AString() = default;

    using super::push_back;

    void push_back(AChar c) noexcept;

    void insert(size_type pos, AChar c);

    void insert(size_type pos, AStringView str);

    /**
     * @brief Encodes the string into a null-terminated byte buffer using the specified encoding.
     * @sa bytes
     */
    AByteBuffer encode(AStringEncoding encoding) const;

    /**
     * @brief Returns a view of the raw UTF-8 encoded byte data.
     * @sa encode
     */
    constexpr bytes_type& bytes() noexcept {
        return *this;
    }

    /**
     * @brief Returns a view of the raw UTF-8 encoded byte data.
     * @sa encode
     */
    constexpr const bytes_type& bytes() const noexcept {
        return *this;
    }

    /**
     * @brief Compatibility method. Guarantees std::string with UTF-8
     * @sa bytes
     */
    std::string& toStdString() {
        return *this;
    }

    /**
     * @brief Compatibility method. Guarantees std::string with UTF-8
     * @sa bytes
     */
    const std::string& toStdString() const {
        return *this;
    }

    operator AStringView() const noexcept;

    AStringView view() const noexcept {
        return this->operator AStringView();
    }

    /**
     * @brief Returns the number of bytes in the UTF-8 encoded string
     * @sa length
     */
    size_type sizeBytes() const noexcept {
        return size();
    }

    /**
     * @brief Returns the number of Unicode characters in the string
     * @sa sizeBytes
     */
    size_type length() const noexcept;

    constexpr AString substr(size_type pos = 0, size_type n = npos) const {
        return AString(super::substr(pos, n));
    }

    AString trimLeft(char symbol = ' ') const;
    AString trimRight(char symbol = ' ') const;
    AString trim(char symbol = ' ') const;

    AString& operator=(const AString& other) {
        std::string& ul = *this;
        ul = other;
        return *this;
    }

    AString& operator=(AString&& other) noexcept {
        std::string& ul = *this;
        ul = std::move(other);
        return *this;
    }

    using super::append;

    AString& append(char c);

    AString& append(AChar c);

    AString& operator<<(char c) noexcept
    {
        append(c);
        return *this;
    }

    AString& operator<<(AChar c) noexcept
    {
        append(c);
        return *this;
    }

    AString uppercase() const;

    AString lowercase() const;

    AStringVector split(AChar c) const;

    AString& replaceAll(char from, char to);

    AString& replaceAll(AStringView from, AStringView to);

    AString replacedAll(AChar from, AChar to) const;

    AString replacedAll(AStringView from, AStringView to) const;

    AString& removeAll(AChar c);

    AString processEscapes() const;

    /**
     * @brief Resizes the string to the length of its null-terminated content while preserving capacity.
     * @sa shrink_to_fit
     */
    void resizeToNullTerminator();

    bool contains(char c) const noexcept;

    bool startsWith(AChar prefix) const noexcept;

    bool startsWith(AStringView prefix) const noexcept {
        return view().startsWith(prefix);
    }

    bool endsWith(AChar prefix) const noexcept;

    bool endsWith(AStringView suffix) const noexcept {
        return view().endsWith(suffix);
    }

    /**
     * @brief Converts the string to boolean value.
     * @return If the string equals to "true", true returned, false otherwise.
     */
    bool toBool() const {
        return sizeBytes() == 4 && lowercase() == "true";
    }

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

    template<typename... Args>
    AString format(Args&&... args) const;

    iterator begin() noexcept {
        return AUtf8MutableIterator(this, 0);
    }

    iterator end() noexcept {
        return AUtf8MutableIterator(this, size());
    }

    const_iterator begin() const noexcept {
        return AUtf8ConstIterator(data(), data(), data() + size(), 0);
    }

    const_iterator end() const noexcept {
        return AUtf8ConstIterator(data(), data(), data() + size(), size());
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    reverse_iterator rbegin() noexcept {
        return AUtf8ConstReverseIterator(end());
    }

    reverse_iterator rend() noexcept {
        return AUtf8ConstReverseIterator(begin());
    }

    const_reverse_iterator rbegin() const noexcept {
        return AUtf8ConstReverseIterator(end());
    }

    const_reverse_iterator rend() const noexcept {
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

    AChar operator[](size_type i) const {
        if (empty()) {
            return AChar();
        }
        return *(begin() + i);
    }

    iterator erase(const_iterator it);

    iterator erase(const_iterator begin, const_iterator end);

    void erase(size_t u_pos, size_t u_count);

//private: // non private because ASerializable
    size_type size() const noexcept {
        return super::size();
    }
};

inline AString operator+(const AString& l, const AString& r) noexcept
{
    return static_cast<const std::string&>(l) + static_cast<const std::string&>(r);
}
inline AString operator+(const AString& l, const std::string& r) noexcept
{
    return static_cast<const std::string&>(l) + r;
}
inline AString operator+(const AString& l, char r) noexcept
{
    auto x = l;
    x.append(r);
    return x;
}
inline AString operator+(const AString& l, AChar r) noexcept
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
        return std::hash<std::string>()(t);
    }
};

#if defined(FMT_VERSION) && (FMT_VERSION < 100000)
template <> struct fmt::detail::is_string<AString>: std::false_type {};
#endif

template <> struct fmt::formatter<AString>: fmt::formatter<std::string> {
    auto format(const AString& s, fmt::format_context& ctx) const {
        return fmt::formatter<std::string>::format(s.toStdString(), ctx);
    }
};

// gtest printer for AString
inline void PrintTo(const AString& s, std::ostream* stream) {
    *stream << s.toStdString();
}
