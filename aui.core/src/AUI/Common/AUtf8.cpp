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

#include "AUtf8.h"

namespace aui::utf8::detail {

API_AUI_CORE char32_t decodeUtf8At(const char* data, size_t& bytePos, size_t maxSize) noexcept {
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

API_AUI_CORE size_t getPrevCharStart(const char* data, size_t pos) noexcept {
    if (pos == 0) return 0;

    size_t prev_pos = pos - 1;

    // Move back while we're in continuation bytes
    while (prev_pos > 0 && (static_cast<unsigned char>(data[prev_pos]) & 0xC0) == 0x80) {
        --prev_pos;
    }

    return prev_pos;
}

inline size_t utf8_char_length(unsigned char first_byte) {
    if ((first_byte & 0x80) == 0) return 1; // 0xxxxxxx
    if ((first_byte & 0xE0) == 0xC0) return 2; // 110xxxxx
    if ((first_byte & 0xF0) == 0xE0) return 3; // 1110xxxx
    if ((first_byte & 0xF8) == 0xF0) return 4; // 11110xxx
    return 0; // Invalid UTF-8
}

API_AUI_CORE std::optional<size_t> findUnicodePos(std::string_view utf8_str, size_t unicode_index) {
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

auto AUtf8ConstIterator::operator-(const AUtf8ConstIterator& other) const noexcept -> difference_type {
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
