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

}

/**
 * @brief UTF-8 forward iterator for AString
 */
class AStringUtf8Iterator {
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
    mutable char32_t cached_value_;
    mutable bool cache_valid_;

public:
    AStringUtf8Iterator() noexcept
        : data_(nullptr), begin_(nullptr), end_(nullptr),
          byte_pos_(0), cached_value_(0), cache_valid_(false) {}

    AStringUtf8Iterator(const char* data, const char* begin, const char* end, size_t pos) noexcept
        : data_(data), begin_(begin), end_(end),
          byte_pos_(pos), cached_value_(0), cache_valid_(false) {}

    char32_t operator*() const noexcept {
        if (!cache_valid_) {
            size_t temp_pos = byte_pos_;
            cached_value_ = aui::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
            cache_valid_ = true;
        }
        return cached_value_;
    }

    AStringUtf8Iterator& operator++() noexcept {
        if (byte_pos_ < static_cast<size_t>(end_ - begin_)) {
            size_t temp_pos = byte_pos_;
            aui::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
            byte_pos_ = temp_pos;
        }
        cache_valid_ = false;
        return *this;
    }

    AStringUtf8Iterator operator++(int) noexcept {
        AStringUtf8Iterator temp = *this;
        ++(*this);
        return temp;
    }

    AStringUtf8Iterator& operator--() noexcept {
        if (byte_pos_ > 0) {
            byte_pos_ = aui::detail::getPrevCharStart(data_, byte_pos_);
        }
        cache_valid_ = false;
        return *this;
    }

    AStringUtf8Iterator operator--(int) noexcept {
        AStringUtf8Iterator temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const AStringUtf8Iterator& other) const noexcept {
        return data_ == other.data_ && byte_pos_ == other.byte_pos_;
    }

    bool operator!=(const AStringUtf8Iterator& other) const noexcept {
        return !(*this == other);
    }

    size_t getBytePos() const noexcept {
        return byte_pos_;
    }
};

/**
 * @brief UTF-8 reverse iterator for AString
 */
class AStringUtf8ReverseIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = const char32_t*;
    using reference = char32_t;

private:
    AStringUtf8Iterator base_iterator_;

public:
    explicit AStringUtf8ReverseIterator() noexcept = default;

    explicit AStringUtf8ReverseIterator(AStringUtf8Iterator it) noexcept
        : base_iterator_(it) {
        --base_iterator_;
    }

    AStringUtf8Iterator base() const noexcept {
        AStringUtf8Iterator temp = base_iterator_;
        ++temp;
        return temp;
    }

    char32_t operator*() const noexcept {
        return *base_iterator_;
    }

    AStringUtf8ReverseIterator& operator++() noexcept {
        --base_iterator_;
        return *this;
    }

    AStringUtf8ReverseIterator operator++(int) noexcept {
        AStringUtf8ReverseIterator temp = *this;
        ++(*this);
        return temp;
    }

    AStringUtf8ReverseIterator& operator--() noexcept {
        ++base_iterator_;
        return *this;
    }

    AStringUtf8ReverseIterator operator--(int) noexcept {
        AStringUtf8ReverseIterator temp = *this;
        --(*this);
        return temp;
    }

    bool operator==(const AStringUtf8ReverseIterator& other) const noexcept {
        return base_iterator_ == other.base_iterator_;
    }

    bool operator!=(const AStringUtf8ReverseIterator& other) const noexcept {
        return !(*this == other);
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

class AStringView;

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

    using utf8_iterator = AStringUtf8Iterator;
    using utf8_const_iterator = AStringUtf8Iterator;
    using utf8_reverse_iterator = AStringUtf8ReverseIterator;
    using utf8_const_reverse_iterator = AStringUtf8ReverseIterator;

    using byte_iterator = super::iterator;
    using byte_const_iterator = super::const_iterator;
    using byte_reverse_iterator = super::reverse_iterator;
    using byte_const_reverse_iterator = super::const_reverse_iterator;

    using iterator = utf8_iterator;
    using const_iterator = utf8_const_iterator;
    using reverse_iterator = utf8_reverse_iterator;
    using const_reverse_iterator = utf8_const_reverse_iterator;

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

    AString(const char16_t* utf16_bytes, size_type length);

    AString(const char16_t* utf16_bytes) : AString(utf16_bytes, strLength(utf16_bytes)) {}

    AString(const char32_t* utf32_bytes, size_type length);

    AString(const char32_t* utf32_bytes) : AString(utf32_bytes, strLength(utf32_bytes)) {}

    explicit AString(AStringView view);

    constexpr AString(super&& other) : super(std::move(other)) {}

    AString(AChar c);

    ~AString() = default;

    using super::push_back;

    void push_back(AChar c) noexcept;

    AByteBuffer getBytes(AStringEncoding encoding) const;

    /// Compatibility method
    std::string toStdString() const {
        return *this;
    }

    operator AStringView() const noexcept;

    /// Returns the number of bytes in the UTF-8 encoded string
    constexpr size_type sizeBytes() const noexcept {
        return size();
    }

    /// Returns the number of Unicode characters in the string
    size_type length() const noexcept;

    constexpr AString substr(size_type pos = 0, size_type n = npos) const {
        return AString(super::substr(pos, n));
    }

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

    AString uppercase() const;

    AString lowercase() const;

    AStringVector split(AChar c) const;

    bool startsWith(AStringView prefix) const noexcept;

    bool endsWith(AStringView suffix) const noexcept;

    bool toBool() const {
        return sizeBytes() == 4 && lowercase() == "true";
    }

    AOptional<int32_t> toInt() const noexcept;

    AOptional<int64_t> toLong() const noexcept;

    AOptional<uint32_t> toUInt() const noexcept;

    AOptional<uint64_t> toULong() const noexcept;

    AOptional<float> toFloat() const noexcept;

    AOptional<double> toDouble() const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Returns std::nullopt if the conversion fails.
     * @sa toNumberOrException
     */
    AOptional<int> toNumber(aui::ranged_number<int, 2, 36> base) const noexcept;

    /**
     * @brief Returns the string converted to an int using base. Throws an exception if the conversion fails.
     * @sa toNumber
     */
    int toNumberOrException(aui::ranged_number<int, 2, 36> base = TO_NUMBER_BASE_DEC) const {
        return toNumber(base).valueOrException(fmt::format("bad to number conversion: {}", toStdString()).c_str());
    }

    template<typename... Args>
    AString format(Args&&... args) const;

    utf8_iterator utf8_begin() noexcept {
        return utf8_iterator(data(), data(), data() + size(), 0);
    }

    utf8_const_iterator utf8_begin() const noexcept {
        return utf8_const_iterator(data(), data(), data() + size(), 0);
    }

    utf8_const_iterator utf8_cbegin() const noexcept {
        return utf8_begin();
    }

    utf8_iterator utf8_end() noexcept {
        return utf8_iterator(data(), data(), data() + size(), size());
    }

    utf8_const_iterator utf8_end() const noexcept {
        return utf8_const_iterator(data(), data(), data() + size(), size());
    }

    utf8_const_iterator utf8_cend() const noexcept {
        return utf8_end();
    }

    utf8_reverse_iterator utf8_rbegin() noexcept {
        return utf8_reverse_iterator(utf8_end());
    }

    utf8_const_reverse_iterator utf8_rbegin() const noexcept {
        return utf8_const_reverse_iterator(utf8_end());
    }

    utf8_const_reverse_iterator utf8_crbegin() const noexcept {
        return utf8_rbegin();
    }

    utf8_reverse_iterator utf8_rend() noexcept {
        return utf8_reverse_iterator(utf8_begin());
    }

    utf8_const_reverse_iterator utf8_rend() const noexcept {
        return utf8_const_reverse_iterator(utf8_begin());
    }

    utf8_const_reverse_iterator utf8_crend() const noexcept {
        return utf8_rend();
    }

    iterator begin() noexcept {
        return utf8_begin();
    }

    const_iterator begin() const noexcept {
        return utf8_begin();
    }

    const_iterator cbegin() const noexcept {
        return utf8_begin();
    }

    iterator end() noexcept {
        return utf8_end();
    }

    const_iterator end() const noexcept {
        return utf8_end();
    }

    const_iterator cend() const noexcept {
        return utf8_end();
    }

    reverse_iterator rbegin() noexcept {
        return utf8_rbegin();
    }

    const_reverse_iterator rbegin() const noexcept {
        return utf8_rbegin();
    }

    const_reverse_iterator crbegin() const noexcept {
        return utf8_rbegin();
    }

    reverse_iterator rend() noexcept {
        return utf8_rend();
    }

    const_reverse_iterator rend() const noexcept {
        return utf8_rend();
    }

    const_reverse_iterator crend() const noexcept {
        return utf8_rend();
    }

    byte_iterator byte_begin() noexcept {
        return super::begin();
    }

    byte_const_iterator byte_begin() const noexcept {
        return super::begin();
    }

    byte_const_iterator byte_cbegin() const noexcept {
        return super::cbegin();
    }

    byte_iterator byte_end() noexcept {
        return super::end();
    }

    byte_const_iterator byte_end() const noexcept {
        return super::end();
    }

    byte_const_iterator byte_cend() const noexcept {
        return super::cend();
    }

    byte_reverse_iterator byte_rbegin() noexcept {
        return super::rbegin();
    }

    byte_const_reverse_iterator byte_rbegin() const noexcept {
        return super::rbegin();
    }

    byte_const_reverse_iterator byte_crbegin() const noexcept {
        return super::crbegin();
    }

    byte_reverse_iterator byte_rend() noexcept {
        return super::rend();
    }

    byte_const_reverse_iterator byte_rend() const noexcept {
        return super::rend();
    }

    byte_const_reverse_iterator byte_crend() const noexcept {
        return super::crend();
    }

private:
    size_type size() const noexcept {
        return super::size();
    }
};

inline AString operator+(const AString& l, const AString& r) noexcept
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

class AStringView: public std::string_view {
private:
    using super = std::string_view;

public:

    using super::super;

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

};

template<>
struct std::hash<AStringView>
{
    size_t operator()(const AStringView& t) const noexcept
    {
        return std::hash<std::string_view>()(t);
    }
};

inline AString::AString(AStringView view) : super(static_cast<std::string_view>(view)) {}

inline bool AString::startsWith(AStringView prefix) const noexcept {
    return static_cast<AStringView>(*this).startsWith(prefix);
}

inline bool AString::endsWith(AStringView suffix) const noexcept {
    return static_cast<AStringView>(*this).endsWith(suffix);
}
