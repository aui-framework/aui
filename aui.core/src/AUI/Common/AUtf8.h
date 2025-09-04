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

namespace aui::detail {

API_AUI_CORE char32_t decodeUtf8At(const char* data, size_t& bytePos, size_t maxSize) noexcept;

API_AUI_CORE size_t getPrevCharStart(const char* data, size_t pos) noexcept;

API_AUI_CORE std::optional<size_t> findUnicodePos(std::string_view utf8_str, size_t unicode_index);

}

/**
 * @brief UTF-8 forward iterator for AString
 */
class API_AUI_CORE AUtf8ConstIterator {
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
            byte_pos_ = &*it - begin_;
        }
        return *this;
    }

    AUtf8ConstIterator& operator=(const std::string::const_iterator& it) noexcept {
        if (begin_ != nullptr) {
            byte_pos_ = &*it - begin_;
        }
        return *this;
    }
};

/**
 * @brief UTF-8 reverse iterator for AString
 */
class API_AUI_CORE AUtf8ConstReverseIterator {
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
