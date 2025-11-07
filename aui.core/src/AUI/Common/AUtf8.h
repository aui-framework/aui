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

#include <AUI/api.h>
#include <AUI/Common/AChar.h>
#include <AUI/Common/AOptional.h>

namespace aui::utf8::detail {

constexpr char32_t decodeUtf8At(const char* data, size_t& bytePos, size_t maxSize) noexcept {
    if (bytePos >= maxSize) return 0;

    unsigned char first = static_cast<unsigned char>(data[bytePos++]);

    if (first <= 0x7F) {
        return first;
    }

    char32_t result = 0;
    int continuation_bytes = 0;

    if ((first & 0xE0) == 0xC0) {
        result = first & 0x1F;
        continuation_bytes = 1;
    } else if ((first & 0xF0) == 0xE0) {
        result = first & 0x0F;
        continuation_bytes = 2;
    } else if ((first & 0xF8) == 0xF0) {
        result = first & 0x07;
        continuation_bytes = 3;
    } else {
        return 0xFFFD;
    }

    for (int i = 0; i < continuation_bytes && bytePos < maxSize; ++i) {
        unsigned char byte = static_cast<unsigned char>(data[bytePos]);
        if ((byte & 0xC0) != 0x80) {
            return 0xFFFD;
        }
        result = (result << 6) | (byte & 0x3F);
        ++bytePos;
    }

    if (continuation_bytes == 1 && result < 0x80) return 0xFFFD;
    if (continuation_bytes == 2 && result < 0x800) return 0xFFFD;
    if (continuation_bytes == 3 && result < 0x10000) return 0xFFFD;
    if (result > 0x10FFFF) return 0xFFFD;
    if (result >= 0xD800 && result <= 0xDFFF) return 0xFFFD;

    return result;
}

constexpr std::size_t getPrevCharStart(const char* data, size_t pos) noexcept {
    if (pos == 0) return 0;

    size_t prev_pos = pos - 1;

    // Move back while we're in continuation bytes
    while (prev_pos > 0 && (static_cast<unsigned char>(data[prev_pos]) & 0xC0) == 0x80) {
        --prev_pos;
    }

    return prev_pos;
}

constexpr std::size_t utf8_char_length(unsigned char first_byte) {
    if ((first_byte & 0x80) == 0) return 1; // 0xxxxxxx
    if ((first_byte & 0xE0) == 0xC0) return 2; // 110xxxxx
    if ((first_byte & 0xF0) == 0xE0) return 3; // 1110xxxx
    if ((first_byte & 0xF8) == 0xF0) return 4; // 11110xxx
    return 0; // Invalid UTF-8
}

constexpr AOptional<std::size_t> findUnicodePos(std::string_view utf8_str, size_t unicode_index) {
    size_t byte_pos = 0;
    size_t char_count = 0;

    while (byte_pos < utf8_str.size()) {
        if (char_count == unicode_index) {
            return byte_pos;
        }

        size_t char_len = utf8_char_length(static_cast<unsigned char>(utf8_str[byte_pos]));

        if (char_len == 0 || byte_pos + char_len > utf8_str.size()) {
            return std::nullopt;
        }

        byte_pos += char_len;
        char_count++;
    }

    if (char_count == unicode_index) {
        return byte_pos;
    }

    return std::nullopt;
}

}

enum class AStringEncoding : uint8_t {
    UTF8 = 0,
    UTF16 = 1,
    UTF32 = 2,
    LATIN1 = 3,
};

/**
 * @brief UTF-8 forward iterator for AString
 */
class API_AUI_CORE AUtf8ConstIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = AChar;
    using difference_type = std::ptrdiff_t;
    using pointer = const AChar*;
    using reference = AChar;

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

    constexpr AChar operator*() const noexcept {
        size_t temp_pos = byte_pos_;
        return aui::utf8::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
    }

    constexpr AUtf8ConstIterator& operator++() noexcept {
        if (byte_pos_ < static_cast<size_t>(end_ - begin_)) {
            size_t temp_pos = byte_pos_;
            aui::utf8::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
            byte_pos_ = temp_pos;
        }
        return *this;
    }

    constexpr const char* data() const noexcept {
        return data_ + byte_pos_;
    }

    constexpr AUtf8ConstIterator operator++(int) noexcept {
        AUtf8ConstIterator temp = *this;
        ++(*this);
        return temp;
    }

    constexpr AUtf8ConstIterator& operator--() noexcept {
        if (byte_pos_ > 0) {
            byte_pos_ = aui::utf8::detail::getPrevCharStart(data_, byte_pos_);
        }
        return *this;
    }

    constexpr AUtf8ConstIterator operator--(int) noexcept {
        AUtf8ConstIterator temp = *this;
        --(*this);
        return temp;
    }

    constexpr AUtf8ConstIterator& operator+=(int n) noexcept {
        if (n > 0) {
            while (n-- > 0) {
                ++(*this);
            }
        } else if (n < 0) {
            while (n++ < 0) {
                --(*this);
            }
        }
        return *this;
    }

    constexpr AUtf8ConstIterator& operator-=(int n) noexcept {
        return *this += (-n);
    }

    constexpr AUtf8ConstIterator operator+(int n) const noexcept {
        AUtf8ConstIterator result = *this;
        result += n;
        return result;
    }

    constexpr AUtf8ConstIterator operator-(int n) const noexcept {
        AUtf8ConstIterator result = *this;
        result -= n;
        return result;
    }

    constexpr bool operator==(const AUtf8ConstIterator& other) const noexcept {
        return data_ == other.data_ && byte_pos_ == other.byte_pos_;
    }

    constexpr bool operator!=(const AUtf8ConstIterator& other) const noexcept {
        return !(*this == other);
    }

    constexpr size_t getBytePos() const noexcept {
        return byte_pos_;
    }

    constexpr AUtf8ConstIterator& operator=(const std::string::iterator& it) noexcept {
        if (begin_ != nullptr) {
            byte_pos_ = &*it - begin_;
        }
        return *this;
    }

    constexpr AUtf8ConstIterator& operator=(const std::string::const_iterator& it) noexcept {
        if (begin_ != nullptr) {
            byte_pos_ = &*it - begin_;
        }
        return *this;
    }

    constexpr difference_type operator-(const AUtf8ConstIterator& other) const noexcept {
        if (data_ != other.data_) {
            return 0;
        }

        if (byte_pos_ == other.byte_pos_) {
            return 0;
        }

        size_t start_pos, end_pos;
        bool forward;

        if (byte_pos_ > other.byte_pos_) {
            start_pos = other.byte_pos_;
            end_pos = byte_pos_;
            forward = true;
        } else {
            start_pos = byte_pos_;
            end_pos = other.byte_pos_;
            forward = false;
        }

        difference_type count = 0;
        size_t current_pos = start_pos;

        while (current_pos < end_pos) {
            size_t temp_pos = current_pos;
            aui::utf8::detail::decodeUtf8At(data_, temp_pos, end_ - begin_);
            current_pos = temp_pos;
            ++count;
        }

        return forward ? count : -count;
    }

    constexpr bool operator<(const AUtf8ConstIterator& other) const noexcept {
        return byte_pos_ < other.byte_pos_;
    }

    constexpr bool operator<=(const AUtf8ConstIterator& other) const noexcept {
        return byte_pos_ <= other.byte_pos_;
    }

    constexpr bool operator>(const AUtf8ConstIterator& other) const noexcept {
        return byte_pos_ > other.byte_pos_;
    }

    constexpr bool operator>=(const AUtf8ConstIterator& other) const noexcept {
        return byte_pos_ >= other.byte_pos_;
    }
};

/**
 * @brief UTF-8 reverse iterator for AString
 */
class API_AUI_CORE AUtf8ConstReverseIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = AChar;
    using difference_type = std::ptrdiff_t;
    using pointer = const AChar*;
    using reference = AChar;

private:
    AUtf8ConstIterator base_iterator_;

public:
    constexpr explicit AUtf8ConstReverseIterator() noexcept = default;

    constexpr explicit AUtf8ConstReverseIterator(AUtf8ConstIterator it) noexcept
        : base_iterator_(it) {
        --base_iterator_;
    }

    constexpr AUtf8ConstIterator base() const noexcept {
        AUtf8ConstIterator temp = base_iterator_;
        ++temp;
        return temp;
    }

    constexpr char32_t operator*() const noexcept {
        return *base_iterator_;
    }

    constexpr AUtf8ConstReverseIterator& operator++() noexcept {
        --base_iterator_;
        return *this;
    }

    constexpr AUtf8ConstReverseIterator operator++(int) noexcept {
        AUtf8ConstReverseIterator temp = *this;
        ++(*this);
        return temp;
    }

    constexpr AUtf8ConstReverseIterator& operator--() noexcept {
        ++base_iterator_;
        return *this;
    }

    constexpr AUtf8ConstReverseIterator operator--(int) noexcept {
        AUtf8ConstReverseIterator temp = *this;
        --(*this);
        return temp;
    }

    constexpr bool operator==(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ == other.base_iterator_;
    }

    constexpr bool operator!=(const AUtf8ConstReverseIterator& other) const noexcept {
        return !(*this == other);
    }

    constexpr AUtf8ConstReverseIterator& operator=(std::string::reverse_iterator it) noexcept {
        base_iterator_ = it.base();
        --base_iterator_;
        return *this;
    }

    constexpr AUtf8ConstReverseIterator& operator=(std::string::const_reverse_iterator it) noexcept {
        base_iterator_ = it.base();
        --base_iterator_;
        return *this;
    }

    constexpr difference_type operator-(const AUtf8ConstReverseIterator& other) const noexcept {
        return other.base_iterator_ - base_iterator_;
    }

    constexpr bool operator<(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ > other.base_iterator_;
    }

    constexpr bool operator<=(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ >= other.base_iterator_;
    }

    constexpr bool operator>(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ < other.base_iterator_;
    }

    constexpr bool operator>=(const AUtf8ConstReverseIterator& other) const noexcept {
        return base_iterator_ <= other.base_iterator_;
    }
};
