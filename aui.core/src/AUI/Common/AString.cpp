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

#include <cstring>
#include "AString.h"
#include "AStringVector.h"
#include "AStaticVector.h"
#include <AUI/Common/AByteBuffer.h>
#include <simdutf.h>

// utf8 stuff has a lot of magic
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)

namespace aui::detail {

char32_t decodeUtf8At(const char* data, size_t& bytePos, size_t maxSize) noexcept {
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

size_t getPrevCharStart(const char* data, size_t pos) noexcept {
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

std::optional<size_t> findUnicodePos(std::string_view utf8_str, size_t unicode_index) {
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

size_t AUtf8MutableIterator::getCurrentCharByteLength() const noexcept {
    if (!string_ || byte_pos_ >= string_->size()) {
        return 0;
    }

    const char* data = string_->data();
    unsigned char first_byte = static_cast<unsigned char>(data[byte_pos_]);

    if (first_byte < 0x80) return 1;      // 0xxxxxxx
    if (first_byte < 0xC0) return 1;      // Invalid continuation byte, treat as 1
    if (first_byte < 0xE0) return 2;      // 110xxxxx
    if (first_byte < 0xF0) return 3;      // 1110xxxx
    if (first_byte < 0xF8) return 4;      // 11110xxx
    return 1; // Invalid, treat as 1
}

size_t AUtf8MutableIterator::getEncodedByteLength(char32_t codepoint) noexcept {
    if (codepoint <= 0x7F) return 1;
    if (codepoint <= 0x7FF) return 2;
    if (codepoint <= 0xFFFF) return 3;
    if (codepoint <= 0x10FFFF) return 4;
    return 3; // Invalid codepoint, encode as replacement character (3 bytes)
}

size_t AUtf8MutableIterator::encodeUtf8(char32_t codepoint, char* buffer) noexcept {
    if (codepoint <= 0x7F) {
        buffer[0] = static_cast<char>(codepoint);
        return 1;
    }
    if (codepoint <= 0x7FF) {
        buffer[0] = static_cast<char>(0xC0 | (codepoint >> 6));
        buffer[1] = static_cast<char>(0x80 | (codepoint & 0x3F));
        return 2;
    }
    if (codepoint <= 0xFFFF) {
        buffer[0] = static_cast<char>(0xE0 | (codepoint >> 12));
        buffer[1] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (codepoint & 0x3F));
        return 3;
    }
    if (codepoint <= 0x10FFFF) {
        buffer[0] = static_cast<char>(0xF0 | (codepoint >> 18));
        buffer[1] = static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (codepoint & 0x3F));
        return 4;
    }

    // Invalid codepoint, encode replacement character U+FFFD
    buffer[0] = static_cast<char>(0xEF);
    buffer[1] = static_cast<char>(0xBF);
    buffer[2] = static_cast<char>(0xBD);
    return 3;
}

AUtf8MutableIterator::AUtf8MutableIterator() noexcept
    : string_(nullptr), byte_pos_(0) {}

AUtf8MutableIterator::AUtf8MutableIterator(AString* str, size_t pos) noexcept
    : string_(str), byte_pos_(pos) {}

AChar AUtf8MutableIterator::operator*() const noexcept {
    if (!string_ || byte_pos_ >= string_->size()) {
        return AChar();
    }

    size_t temp_pos = byte_pos_;
    return aui::detail::decodeUtf8At(string_->data(), temp_pos, string_->size());
}

AUtf8MutableIterator& AUtf8MutableIterator::operator=(AChar c) {
    if (!string_ || byte_pos_ >= string_->size()) {
        return *this;
    }

    char32_t new_codepoint = c.codepoint();
    size_t current_char_bytes = getCurrentCharByteLength();
    size_t new_char_bytes = getEncodedByteLength(new_codepoint);

    char utf8_buffer[4];
    size_t encoded_bytes = encodeUtf8(new_codepoint, utf8_buffer);

    if (current_char_bytes == new_char_bytes) {
        // Same byte length - simple replacement
        std::memcpy(string_->data() + byte_pos_, utf8_buffer, encoded_bytes);
    } else if (current_char_bytes > new_char_bytes) {
        // New character is shorter - replace and shift left
        std::memcpy(string_->data() + byte_pos_, utf8_buffer, encoded_bytes);

        size_t shift_start = byte_pos_ + current_char_bytes;
        size_t shift_end = string_->size();
        size_t bytes_to_shift = shift_end - shift_start;

        if (bytes_to_shift > 0) {
            std::memmove(string_->data() + byte_pos_ + encoded_bytes,
                       string_->data() + shift_start,
                       bytes_to_shift);
        }

        string_->resize(string_->size() - (current_char_bytes - encoded_bytes));
    } else {
        // New character is longer - need to make space and shift right
        size_t bytes_to_add = new_char_bytes - current_char_bytes;
        size_t old_size = string_->size();
        string_->resize(old_size + bytes_to_add);

        size_t shift_start = byte_pos_ + current_char_bytes;
        size_t bytes_to_shift = old_size - shift_start;

        if (bytes_to_shift > 0) {
            std::memmove(string_->data() + byte_pos_ + encoded_bytes,
                       string_->data() + shift_start,
                       bytes_to_shift);
        }

        std::memcpy(string_->data() + byte_pos_, utf8_buffer, encoded_bytes);
    }

    return *this;
}

AUtf8MutableIterator& AUtf8MutableIterator::operator++() noexcept {
    if (string_ && byte_pos_ < string_->size()) {
        size_t temp_pos = byte_pos_;
        aui::detail::decodeUtf8At(string_->data(), temp_pos, string_->size());
        byte_pos_ = temp_pos;
    }
    return *this;
}

AUtf8MutableIterator AUtf8MutableIterator::operator++(int) noexcept {
    AUtf8MutableIterator temp = *this;
    ++(*this);
    return temp;
}

AUtf8MutableIterator& AUtf8MutableIterator::operator--() noexcept {
    if (string_ && byte_pos_ > 0) {
        byte_pos_ = aui::detail::getPrevCharStart(string_->data(), byte_pos_);
    }
    return *this;
}

AUtf8MutableIterator AUtf8MutableIterator::operator--(int) noexcept {
    AUtf8MutableIterator temp = *this;
    --(*this);
    return temp;
}

AUtf8MutableIterator& AUtf8MutableIterator::operator+=(int n) noexcept {
    if (n > 0) {
        for (int i = 0; i < n && string_ && byte_pos_ < string_->size(); ++i) {
            ++(*this);
        }
    } else if (n < 0) {
        for (int i = 0; i > n && string_ && byte_pos_ > 0; --i) {
            --(*this);
        }
    }
    return *this;
}

bool AUtf8MutableIterator::operator==(const AUtf8MutableIterator& other) const noexcept {
    return string_ == other.string_ && byte_pos_ == other.byte_pos_;
}

bool AUtf8MutableIterator::operator!=(const AUtf8MutableIterator& other) const noexcept {
    return !(*this == other);
}

size_t AUtf8MutableIterator::getBytePos() const noexcept {
    return byte_pos_;
}

AString* AUtf8MutableIterator::getString() const noexcept {
    return string_;
}

AUtf8MutableIterator::operator AUtf8ConstIterator() const noexcept {
    if (!string_) {
        return AUtf8ConstIterator();
    }
    return AUtf8ConstIterator(string_->data(), string_->data(),
                             string_->data() + string_->size(), byte_pos_);
}

AString AString::numberHex(int i) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%x", static_cast<unsigned>(i));
    return buf;
}

AString::AString(const char* bytes, size_t size_bytes, AStringEncoding encoding) {
    switch (encoding) {
        case AStringEncoding::UTF8: {
            super::resize(size_bytes);
            std::memcpy(data(), bytes, size_bytes);
        } break;
        case AStringEncoding::UTF16: {
            size_t size = simdutf::utf8_length_from_utf16(reinterpret_cast<const char16_t*>(bytes), size_bytes / sizeof(char16_t));
            super::resize(size);
            super::resize(simdutf::convert_utf16_to_utf8(reinterpret_cast<const char16_t*>(bytes), size_bytes / sizeof(char16_t), data()));
        } break;
        case AStringEncoding::UTF32: {
            size_t size = simdutf::utf8_length_from_utf32(reinterpret_cast<const char32_t*>(bytes), size_bytes / sizeof(char32_t));
            super::resize(size);
            super::resize(simdutf::convert_utf32_to_utf8(reinterpret_cast<const char32_t*>(bytes), size_bytes / sizeof(char32_t), data()));
        } break;
        case AStringEncoding::LATIN1: {
            size_t size = simdutf::utf8_length_from_latin1(reinterpret_cast<const char*>(bytes), size_bytes);
            super::resize(size);
            super::resize(simdutf::convert_latin1_to_utf8(reinterpret_cast<const char*>(bytes), size_bytes, data()));
        } break;
    }
}

AString::AString(const AByteBuffer& buffer, AStringEncoding encoding) : AString((const char*) buffer.data(), buffer.size(), encoding) {}

AString::AString(const char* utf8_bytes, size_type length) {
    if (simdutf::validate_utf8(utf8_bytes, length)) {
        *this = std::string(utf8_bytes, length);
    } else {
        *this = AString(length, AChar(AChar::INVALID_CHAR));
    }
}

AString::AString(const char16_t* utf16_bytes, size_type length) {
    size_t size = simdutf::utf8_length_from_utf16(utf16_bytes, length);
    super::resize(size);
    super::resize(simdutf::convert_utf16_to_utf8(utf16_bytes, length, data()));
}

AString::AString(const char32_t* utf32_bytes, size_type length) {
    size_t size = simdutf::utf8_length_from_utf32(utf32_bytes, length);
    super::resize(size);
    super::resize(simdutf::convert_utf32_to_utf8(utf32_bytes, length, data()));
}

AString::AString(AChar c) {
    push_back(c);
}

AString::AString(size_type n, AChar c) {
    auto utf8c = c.toUtf8();
    reserve(utf8c.size() * n);
    for (size_t i = 0; i < n; i++) {
        super::append(utf8c.begin(), utf8c.begin() + utf8c.size());
    }
}

void AString::push_back(AChar c) noexcept {
    append(c);
}

void AString::insert(size_type pos, AChar c) {
    auto utf8c = c.toUtf8();
    bytes().insert(bytes().begin() + aui::detail::findUnicodePos(bytes(), pos).value(), utf8c.begin(), utf8c.end());
}

void AString::insert(size_type pos, AStringView str) {
    bytes().insert(bytes().begin() + aui::detail::findUnicodePos(bytes(), pos).value(), str.begin(), str.end());
}

AByteBuffer AString::encode(AStringEncoding encoding) const {
    AByteBuffer bytes;
    if (super::empty()) return bytes;
    switch (encoding) {
        case AStringEncoding::UTF8: {
            bytes.reserve(super::size() + 1);
            bytes.write(super::data(), super::size());
            bytes.data()[bytes.capacity() - 1] = '\0';
        } break;
        case AStringEncoding::UTF16: {
            size_t words = simdutf::utf16_length_from_utf8(super::data(), super::size());
            bytes.resize((words + 1) * sizeof(char16_t));
            bytes.resize(simdutf::convert_utf8_to_utf16(super::data(), super::size(), reinterpret_cast<char16_t*>(bytes.data())) * sizeof(char16_t) + sizeof(char16_t));
            reinterpret_cast<char16_t*>(bytes.data())[words] = '\0';
        } break;
        case AStringEncoding::UTF32: {
            size_t words = simdutf::utf32_length_from_utf8(super::data(), super::size());
            bytes.resize((words + 1) * sizeof(char32_t));
            bytes.resize(simdutf::convert_utf8_to_utf32(super::data(), super::size(), reinterpret_cast<char32_t*>(bytes.data())) * sizeof(char32_t) + sizeof(char32_t));
            reinterpret_cast<char32_t*>(bytes.data())[words] = '\0';
        } break;
        case AStringEncoding::LATIN1: {
            size_t words = simdutf::latin1_length_from_utf8(super::data(), super::size());
            bytes.resize(words + 1);
            bytes.resize(simdutf::convert_utf8_to_latin1(super::data(), super::size(), reinterpret_cast<char*>(bytes.data())) + 1);
            bytes.data()[bytes.capacity() - 1] = '\0';
        } break;
    }
    return std::move(bytes);
}

AString::operator AStringView() const noexcept {
    return {bytes().data(), bytes().size()};
}

AString::size_type AString::length() const noexcept {
    return simdutf::count_utf8(super::data(), super::size());
}

AString AString::trimLeft(char symbol) const
{
    for (auto i = begin(); i != end(); ++i)
    {
        if (*i != symbol)
        {
            return { i, end() };
        }
    }
    return {};
}

AString AString::trimRight(char symbol) const
{
    for (auto i = rbegin(); i != rend(); ++i)
    {
        if (*i != symbol)
        {
            return { begin(), i.base() };
        }
    }
    return {};
}

AString AString::trim(char symbol) const
{
    auto left = begin();
    auto right = end();

    while (left != right && *left == symbol)
    {
        ++left;
    }

    if (left != right)
    {
        auto riter = rbegin();
        while (riter.base() != left && *riter == symbol)
        {
            ++riter;
        }
        right = riter.base();
    }

    return { left, right };
}

/*AString AString::restrictLength(size_t s, const AString& stringAtEnd) const {
    if (length() > s) {
        return substr(0, s) + stringAtEnd;
    }
    return *this;
}

AString AString::excessSpacesRemoved() const noexcept {
    AString s;
    s.reserve(length() + 1);
    bool prevWasSpace = false;
    for (auto c : *this) {
        if (c == ' ') {
            if (prevWasSpace) {
                continue;
            }
            prevWasSpace = true;
        } else {
            prevWasSpace = false;
        }
        s << c;
    }
    return s;
}*/

AString& AString::append(char c) {
    super::append(&c, 1);
    return *this;
}

AString& AString::append(AChar c) {
    auto utf8c = c.toUtf8();
    super::append(utf8c.begin(), utf8c.end());
    return *this;
}

AString AString::uppercase() const {
    std::string buf = toStdString();
    {
        auto p = reinterpret_cast<unsigned char *>(buf.data());
        unsigned char* pExtChar = 0;
        while (*p) {
            if ((*p >= 0x61) && (*p <= 0x7a)) // US ASCII
                (*p) -= 0x20;
            else if (*p > 0xc0) {
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                    case 0xc3: // Latin 1
                        if ((*p >= 0xa0)
                            && (*p <= 0xbe)
                            && (*p != 0xb7))
                            (*p) -= 0x20; // US ASCII shift
                        break;
                    case 0xc4: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0xb7)
                            && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0xb9)
                                 && (*p <= 0xbe)
                                 && (!(*p % 2))) // Even
                            (*p)--; // Prev char is upr
                        else if (*p == 0xbf) {
                            *pExtChar = 0xc5;
                            (*p) = 0x80;
                        }
                        break;
                    case 0xc5: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0x88)
                            && (!(*p % 2))) // Even
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0x8a)
                                 && (*p <= 0xb7)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0xb9)
                                 && (*p <= 0xbe)
                                 && (!(*p % 2))) // Even
                            (*p)--; // Prev char is upr
                        break;
                    case 0xc6: // Latin Exteneded
                        switch (*p) {
                            case 0x83:
                            case 0x85:
                            case 0x88:
                            case 0x8c:
                            case 0x92:
                            case 0x99:
                            case 0xa1:
                            case 0xa3:
                            case 0xa5:
                            case 0xa8:
                            case 0xad:
                            case 0xb0:
                            case 0xb4:
                            case 0xb6:
                            case 0xb9:
                            case 0xbd:
                                (*p)--; // Prev char is upr
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0xc7: // Latin Exteneded
                        if (*p == 0x86)
                            (*p) = 0x84;
                        else if (*p == 0x85)
                            (*p)--; // Prev char is upr
                        else if (*p == 0x89)
                            (*p) = 0x87;
                        else if (*p == 0x88)
                            (*p)--; // Prev char is upr
                        else if (*p == 0x8c)
                            (*p) = 0x8a;
                        else if (*p == 0x8b)
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0x8d)
                                 && (*p <= 0x9c)
                                 && (!(*p % 2))) // Even
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0x9e)
                                 && (*p <= 0xaf)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if (*p == 0xb3)
                            (*p) = 0xb1;
                        else if (*p == 0xb2)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xb4)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xb8)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xba)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xbc)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xbe)
                            (*p)--; // Prev char is upr
                        break;
                    case 0xc8: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0x9f)
                            && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0xa2)
                                 && (*p <= 0xb3)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if (*p == 0xbc)
                            (*p)--; // Prev char is upr
                        break;
                    case 0xcd: // Greek & Coptic
                        switch (*p) {
                            case 0xb1:
                            case 0xb3:
                            case 0xb7:
                                (*p)--; // Prev char is upr
                                break;
                            default:
                                if (*p == 0xbb) {
                                    *pExtChar = 0xcf;
                                    (*p) = 0xbd;
                                }
                                else if (*p == 0xbc) {
                                    *pExtChar = 0xcf;
                                    (*p) = 0xbe;
                                }
                                else if (*p == 0xbd) {
                                    *pExtChar = 0xcf;
                                    (*p) = 0xbf;
                                }
                                break;
                        }
                        break;
                    case 0xce: // Greek & Coptic
                        if (*p == 0xac)
                            (*p) = 0x86;
                        else if (*p == 0xad)
                            (*p) = 0x88;
                        else if (*p == 0xae)
                            (*p) = 0x89;
                        else if (*p == 0xaf)
                            (*p) = 0x8a;
                        else if ((*p >= 0xb1)
                                 && (*p <= 0xbf))
                            (*p) -= 0x20; // US ASCII shift
                        break;
                    case 0xcf: // Greek & Coptic
                        if (*p == 0xb4)
                            (*p) = 0x8f;
                        else if (*p == 0x92)
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0x98)
                                 && (*p <= 0xaf)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if (*p == 0x91)
                            (*p) = 0xb4;
                        else if (*p == 0xb8)
                            (*p)--; // Prev char is upr
                        else if (*p == 0xb2)
                            (*p) = 0xb9;
                        else if (*p == 0xbc)
                            (*p)--; // Prev char is upr
                        else if (*p == 0x8c) {
                            *pExtChar = 0xce;
                            (*p) = 0x8c;
                        }
                        else if (*p == 0x8d) {
                            *pExtChar = 0xce;
                            (*p) = 0x8e;
                        }
                        else if (*p == 0x8e) {
                            *pExtChar = 0xce;
                            (*p) = 0x8f;
                        }
                        else if ((*p >= 0x80)
                                 && (*p <= 0x8b)
                                 && (*p != 0x82)) {
                            *pExtChar = 0xce;
                            (*p) += 0x20;
                        }
                        else if (*p == 0xb3) {
                            *pExtChar = 0xcd;
                            (*p) = 0xbf;
                        }
                        break;
                    case 0xd0: // Cyrillic
                        if ((*p >= 0xb0)
                            && (*p <= 0xbf))
                            (*p) -= 0x20; // US ASCII shift
                        break;
                    case 0xd1: // Cyrillic supplement
                        if ((*p >= 0x90)
                            && (*p <= 0x9f)) {
                            *pExtChar = 0xd0;
                            (*p) -= 0x10;
                        }
                        else if ((*p >= 0x80)
                                 && (*p <= 0x8f)) {
                            *pExtChar = 0xd0;
                            (*p) += 0x20;
                        }
                        else if ((*p >= 0xa0)
                                 && (*p <= 0xbf)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        break;
                    case 0xd2: // Cyrillic supplement
                        if (*p == 0x80)
                            (*p)++; // Prev char is upr
                        else if ((*p >= 0x8a)
                                 && (*p <= 0xbf)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        break;
                    case 0xd3: // Cyrillic supplement
                        if ((*p >= 0x81)
                            && (*p <= 0x8e)
                            && (!(*p % 2))) // Even
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0x90)
                                 && (*p <= 0xbf)
                                 && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        break;
                    case 0xd4: // Cyrillic supplement & Armenian
                        if ((*p >= 0x80)
                            && (*p <= 0xaf)
                            && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        break;
                    case 0xd5: // Armenian
                        if ((*p >= 0x80)
                            && (*p <= 0x96)
                            && (*p % 2)) // Odd
                            (*p)--; // Prev char is upr
                        else if ((*p >= 0xa1)
                                 && (*p <= 0xaf)) {
                            *pExtChar = 0xd4;
                            (*p) += 0x10;
                        }
                        break;
                    case 0xe1: // Three byte code
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                            case 0x82: // Georgian
                                break;
                            case 0x83: // Georgian
                                if ((*p >= 0x90)
                                    && (*p <= 0xaf)) {
                                    *pExtChar = 0x82;
                                    (*p) += 0x10;
                                }
                                else if (((*p >= 0xb0)
                                         && ((*p <= 0xb5)
                                             || (*p == 0xb7)))
                                         || (*p == 0xbd))
                                    (*p) -= 0x30;
                                break;
                            case 0xb8: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (*p % 2)) // Odd
                                    (*p)--; // Prev char is upr
                                break;
                            case 0xb9: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (*p % 2)) // Odd
                                    (*p)--; // Prev char is upr
                                break;
                            case 0xba: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x94)
                                    && (*p % 2)) // Odd
                                    (*p)--; // Prev char is upr
                                else if ((*p >= 0x9e)
                                         && (*p <= 0xbf)
                                         && (*p % 2)) // Odd
                                    (*p)--; // Prev char is upr
                                break;
                            case 0xbb: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (*p % 2)) // Odd
                                    (*p)--; // Prev char is upr
                                break;
                            case 0xbc: // Greek extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x87))
                                    (*p) += 0x08;
                                else if ((*p >= 0x90)
                                         && (*p <= 0x97))
                                    (*p) += 0x08;
                                else if ((*p >= 0xa0)
                                         && (*p <= 0xa7))
                                    (*p) += 0x08;
                                else if ((*p >= 0xb0)
                                         && (*p <= 0x87))
                                    (*p) += 0x08;
                                break;
                            case 0xbd: // Greek extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x87))
                                    (*p) += 0x08;
                                else if ((*p >= 0x90)
                                         && (*p <= 0x97))
                                    (*p) += 0x08;
                                else if ((*p >= 0xa0)
                                         && (*p <= 0xa7))
                                    (*p) += 0x08;
                                else if ((*p >= 0xb0)
                                         && (*p <= 0x87))
                                    (*p) += 0x08;
                                break;
                            case 0xbe: // Greek extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x87))
                                    (*p) += 0x08;
                                else if ((*p >= 0x90)
                                         && (*p <= 0x97))
                                    (*p) += 0x08;
                                else if ((*p >= 0xa0)
                                         && (*p <= 0xa7))
                                    (*p) += 0x08;
                                else if ((*p >= 0xb0)
                                         && (*p <= 0xb1))
                                    (*p) += 0x08;
                                break;
                            case 0xbf: // Greek extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x84))
                                    (*p) += 0x08;
                                else if ((*p >= 0x90)
                                         && (*p <= 0x93))
                                    (*p) += 0x08;
                                else if ((*p >= 0xa0)
                                         && (*p <= 0xa4))
                                    (*p) += 0x08;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0xf0: // Four byte code
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                            case 0x90:
                                pExtChar = p;
                                p++;
                                switch (*pExtChar) {
                                    case 0x92: // Osage
                                        break;
                                    case 0x93: // Osage
                                        if ((*p >= 0x80)
                                            && (*p <= 0x93))
                                            (*p) += 0x18;
                                        else if ((*p >= 0x98)
                                                 && (*p <= 0xa7)) {
                                            *pExtChar = 0x92;
                                            (*p) += 0x18;
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            case 0x9E:
                                pExtChar = p;
                                p++;
                                switch (*pExtChar) {
                                    case 0xA4: // Adlam
                                        if ((*p >= 0xa2)
                                            && (*p <= 0xc3))
                                            (*p) -= 0x22;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                pExtChar = 0;
            }
            p++;
        }
    }

    return buf;
}

AString AString::lowercase() const {
    std::string buf = toStdString();
    {
        auto p = reinterpret_cast<unsigned char *>(buf.data());
        unsigned char* pExtChar = 0;
        while (*p) {
            if ((*p >= 0x41) && (*p <= 0x5a)) // US ASCII
                (*p) += 0x20;
            else if (*p > 0xc0) {
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                    case 0xc3: // Latin 1
                        if ((*p >= 0x80)
                            && (*p <= 0x9e)
                            && (*p != 0x97))
                            (*p) += 0x20; // US ASCII shift
                        break;
                    case 0xc4: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0xb7)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0xb9)
                                 && (*p <= 0xbe)
                                 && (*p % 2)) // Odd
                            (*p)++; // Next char is lwr
                        else if (*p == 0xbf) {
                            *pExtChar = 0xc5;
                            (*p) = 0x80;
                        }
                        break;
                    case 0xc5: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0x88)
                            && (*p % 2)) // Odd
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x8a)
                                 && (*p <= 0xb7)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0xb9)
                                 && (*p <= 0xbe)
                                 && (*p % 2)) // Odd
                            (*p)++; // Next char is lwr
                        break;
                    case 0xc6: // Latin Exteneded
                        switch (*p) {
                            case 0x82:
                            case 0x84:
                            case 0x87:
                            case 0x8b:
                            case 0x91:
                            case 0x98:
                            case 0xa0:
                            case 0xa2:
                            case 0xa4:
                            case 0xa7:
                            case 0xac:
                            case 0xaf:
                            case 0xb3:
                            case 0xb5:
                            case 0xb8:
                            case 0xbc:
                                (*p)++; // Next char is lwr
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0xc7: // Latin Exteneded
                        if (*p == 0x84)
                            (*p) = 0x86;
                        else if (*p == 0x85)
                            (*p)++; // Next char is lwr
                        else if (*p == 0x87)
                            (*p) = 0x89;
                        else if (*p == 0x88)
                            (*p)++; // Next char is lwr
                        else if (*p == 0x8a)
                            (*p) = 0x8c;
                        else if (*p == 0x8b)
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x8d)
                                 && (*p <= 0x9c)
                                 && (*p % 2)) // Odd
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x9e)
                                 && (*p <= 0xaf)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if (*p == 0xb1)
                            (*p) = 0xb3;
                        else if (*p == 0xb2)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xb4)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xb8)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xba)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xbc)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xbe)
                            (*p)++; // Next char is lwr
                        break;
                    case 0xc8: // Latin Exteneded
                        if ((*p >= 0x80)
                            && (*p <= 0x9f)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0xa2)
                                 && (*p <= 0xb3)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if (*p == 0xbb)
                            (*p)++; // Next char is lwr
                        break;
                    case 0xcd: // Greek & Coptic
                        switch (*p) {
                            case 0xb0:
                            case 0xb2:
                            case 0xb6:
                                (*p)++; // Next char is lwr
                                break;
                            default:
                                if (*p == 0xbf) {
                                    *pExtChar = 0xcf;
                                    (*p) = 0xb3;
                                }
                                break;
                        }
                        break;
                    case 0xce: // Greek & Coptic
                        if (*p == 0x86)
                            (*p) = 0xac;
                        else if (*p == 0x88)
                            (*p) = 0xad;
                        else if (*p == 0x89)
                            (*p) = 0xae;
                        else if (*p == 0x8a)
                            (*p) = 0xaf;
                        else if (*p == 0x8c) {
                            *pExtChar = 0xcf;
                            (*p) = 0x8c;
                        }
                        else if (*p == 0x8e) {
                            *pExtChar = 0xcf;
                            (*p) = 0x8d;
                        }
                        else if (*p == 0x8f) {
                            *pExtChar = 0xcf;
                            (*p) = 0x8e;
                        }
                        else if ((*p >= 0x91)
                                 && (*p <= 0x9f))
                            (*p) += 0x20; // US ASCII shift
                        else if ((*p >= 0xa0)
                                 && (*p <= 0xab)
                                 && (*p != 0xa2)) {
                            *pExtChar = 0xcf;
                            (*p) -= 0x20;
                        }
                        break;
                    case 0xcf: // Greek & Coptic
                        if (*p == 0x8f)
                            (*p) = 0xb4;
                        else if (*p == 0x91)
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x98)
                                 && (*p <= 0xaf)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if (*p == 0xb4)
                            (*p) = 0x91;
                        else if (*p == 0xb7)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xb9)
                            (*p) = 0xb2;
                        else if (*p == 0xbb)
                            (*p)++; // Next char is lwr
                        else if (*p == 0xbd) {
                            *pExtChar = 0xcd;
                            (*p) = 0xbb;
                        }
                        else if (*p == 0xbe) {
                            *pExtChar = 0xcd;
                            (*p) = 0xbc;
                        }
                        else if (*p == 0xbf) {
                            *pExtChar = 0xcd;
                            (*p) = 0xbd;
                        }

                        break;
                    case 0xd0: // Cyrillic
                        if ((*p >= 0x80)
                            && (*p <= 0x8f)) {
                            *pExtChar = 0xd1;
                            (*p) += 0x10;
                        }
                        else if ((*p >= 0x90)
                                 && (*p <= 0x9f))
                            (*p) += 0x20; // US ASCII shift
                        else if ((*p >= 0xa0)
                                 && (*p <= 0xaf)) {
                            *pExtChar = 0xd1;
                            (*p) -= 0x20;
                        }
                        break;
                    case 0xd1: // Cyrillic supplement
                        if ((*p >= 0xa0)
                            && (*p <= 0xbf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xd2: // Cyrillic supplement
                        if (*p == 0x80)
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x8a)
                                 && (*p <= 0xbf)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xd3: // Cyrillic supplement
                        if ((*p >= 0x81)
                            && (*p <= 0x8e)
                            && (*p % 2)) // Odd
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0x90)
                                 && (*p <= 0xbf)
                                 && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xd4: // Cyrillic supplement & Armenian
                        if ((*p >= 0x80)
                            && (*p <= 0xaf)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        else if ((*p >= 0xb1)
                                 && (*p <= 0xbf)) {
                            *pExtChar = 0xd5;
                            (*p) -= 0x10;
                        }
                        break;
                    case 0xd5: // Armenian
                        if ((*p >= 0x80)
                            && (*p <= 0x96)
                            && (!(*p % 2))) // Even
                            (*p)++; // Next char is lwr
                        break;
                    case 0xe1: // Three byte code
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                            case 0x82: // Georgian
                                if ((*p >= 0xa0)
                                    && (*p <= 0xbf)) {
                                    *pExtChar = 0x83;
                                    (*p) -= 0x10;
                                }
                                break;
                            case 0x83: // Georgian
                                if (((*p >= 0x80)
                                    && ((*p <= 0x85)
                                        || (*p == 0x87)))
                                    || (*p == 0x8d))
                                    (*p) += 0x30;
                                break;
                            case 0xb8: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (!(*p % 2))) // Even
                                    (*p)++; // Next char is lwr
                                break;
                            case 0xb9: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (!(*p % 2))) // Even
                                    (*p)++; // Next char is lwr
                                break;
                            case 0xba: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0x94)
                                    && (!(*p % 2))) // Even
                                    (*p)++; // Next char is lwr
                                else if ((*p >= 0x9e)
                                         && (*p <= 0xbf)
                                         && (!(*p % 2))) // Even
                                    (*p)++; // Next char is lwr
                                break;
                            case 0xbb: // Latin extened
                                if ((*p >= 0x80)
                                    && (*p <= 0xbf)
                                    && (!(*p % 2))) // Even
                                    (*p)++; // Next char is lwr
                                break;
                            case 0xbc: // Greek extened
                                if ((*p >= 0x88)
                                    && (*p <= 0x8f))
                                    (*p) -= 0x08;
                                else if ((*p >= 0x98)
                                         && (*p <= 0x9f))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xa8)
                                         && (*p <= 0xaf))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xb8)
                                         && (*p <= 0x8f))
                                    (*p) -= 0x08;
                                break;
                            case 0xbd: // Greek extened
                                if ((*p >= 0x88)
                                    && (*p <= 0x8d))
                                    (*p) -= 0x08;
                                else if ((*p >= 0x98)
                                         && (*p <= 0x9f))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xa8)
                                         && (*p <= 0xaf))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xb8)
                                         && (*p <= 0x8f))
                                    (*p) -= 0x08;
                                break;
                            case 0xbe: // Greek extened
                                if ((*p >= 0x88)
                                    && (*p <= 0x8f))
                                    (*p) -= 0x08;
                                else if ((*p >= 0x98)
                                         && (*p <= 0x9f))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xa8)
                                         && (*p <= 0xaf))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xb8)
                                         && (*p <= 0xb9))
                                    (*p) -= 0x08;
                                break;
                            case 0xbf: // Greek extened
                                if ((*p >= 0x88)
                                    && (*p <= 0x8c))
                                    (*p) -= 0x08;
                                else if ((*p >= 0x98)
                                         && (*p <= 0x9b))
                                    (*p) -= 0x08;
                                else if ((*p >= 0xa8)
                                         && (*p <= 0xac))
                                    (*p) -= 0x08;
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0xf0: // Four byte code
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                            case 0x90:
                                pExtChar = p;
                                p++;
                                switch (*pExtChar) {
                                    case 0x92: // Osage
                                        if ((*p >= 0xb0)
                                            && (*p <= 0xbf)) {
                                            *pExtChar = 0x93;
                                            (*p) -= 0x18;
                                        }
                                        break;
                                    case 0x93: // Osage
                                        if ((*p >= 0x80)
                                            && (*p <= 0x93))
                                            (*p) += 0x18;
                                        break;
                                    default:
                                        break;
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    case 0x9E:
                        pExtChar = p;
                        p++;
                        switch (*pExtChar) {
                            case 0xA4: // Adlam
                                if ((*p >= 0x80)
                                    && (*p <= 0xA1))
                                    (*p) += 0x22;
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                pExtChar = 0;
            }
            p++;
        }
    }

    return buf;
}

AStringVector AString::split(AChar c) const {
    if (empty()) {
        return {};
    }
    auto utf8c = c.toUtf8();
    if (utf8c.empty()) return {};
    std::string separator_utf8(utf8c.begin(), utf8c.end());
    AStringVector result;
    result.reserve(length() / 10);
    for (size_type s = 0;;) {
        auto next = super::find(separator_utf8, s);
        if (next == npos) {
            result << substr(s);
            break;
        }

        result << substr(s, next - s);
        s = next + separator_utf8.length();
    }
    return result;
}

AString& AString::replaceAll(char from, char to) {
    if (empty()) return *this;
    for (size_t i = 0; i < sizeBytes(); ++i) {
        if ((*this)[i] == from) {
            (*this)[i] = to;
        }
    }
    return *this;
}

AString& AString::replaceAll(AStringView from, AStringView to) {
    if (empty()) return *this;
    for (size_type next = 0;;)
    {
        next = find(from, next);
        if (next == NPOS)
        {
            return *this;
        }

        auto fromLength = from.size();
        auto toLength = to.size();

        if (fromLength == toLength) {
            for (auto c : to) {
                *(bytes().begin() + next++) = c;
            }
        } else if (fromLength < toLength) {
            const auto diff = toLength - fromLength;
            for (auto c : aui::range(to.bytes().begin(), to.bytes().end() - diff)) {
                *(bytes().begin() + next++) = c;
            }
            next = std::distance(bytes().begin(), bytes().insert(bytes().begin() + next, to.bytes().begin() + fromLength, to.bytes().end())) + 1;
        } else {
            for (auto c : to) {
                *(bytes().begin() + next++) = c;
            }
            const auto diff = fromLength - toLength;
            super::erase(bytes().begin() + next, bytes().begin() + next + diff);
        }
    }
    return *this;
}

AString AString::replacedAll(AChar from, AChar to) const {
    if (empty()) return {};
    AString copy;
    copy.reserve(sizeBytes());
    for (auto c : *this) {
        if (c == from) {
            copy << to;
        } else {
            copy << c;
        }
    }
    return copy;
}

AString AString::replacedAll(AStringView from, AStringView to) const {
    AString result;

    result.reserve(size());

    for (size_type pos = 0;;)
    {
        auto next = find(from, pos);
        if (next == NPOS)
        {
            result.bytes().insert(result.bytes().end(), bytes().begin() + pos, bytes().end());
            return result;
        }

        result.bytes().insert(result.bytes().end(), bytes().begin() + pos, bytes().begin() + next);
        result.bytes().insert(result.bytes().end(), to.bytes().begin(), to.bytes().end());

        pos = next + from.length();
    }

    return result;
}

AString& AString::removeAll(AChar c) {
    for (auto it = begin(); it != end();) {
        if (*it == c) {
            it = erase(it);
        } else {
            ++it;
        }
    }
    return *this;
}

AString AString::processEscapes() const {
    AString result;
    result.reserve(length());
    bool doEscape = false;
    for (auto& c : static_cast<const std::string&>(*this)) {
        if (doEscape) {
            doEscape = false;
            switch (c) {
                case '\\':
                    result << '\\';
                    break;
                case 'n':
                    result << '\n';
                    break;
                default:
                    result << c;
            }
        } else if (c == '\\') {
            doEscape = true;
        } else {
            result << c;
        }
    }
    return result;
}

void AString::resizeToNullTerminator() {
    char* current = data();
    char* end = current;
    while (*end != '\0') {
        ++end;
    }
    resize(end - current);
}

bool AString::contains(char c) const noexcept {
    return find(c) != npos;
}

bool AString::contains(AStringView str) const noexcept {
    if (str.empty()) return true;
    if (str.length() > length()) return false;
    return std::search(bytes().begin(), bytes().end(), str.bytes().begin(), str.bytes().end()) != bytes().end();
}

bool AString::startsWith(AChar prefix) const noexcept {
    auto utf8p = prefix.toUtf8();
    return startsWith(AStringView(utf8p.begin(), utf8p.end()));
}

bool AString::endsWith(AChar suffix) const noexcept {
    auto utf8s = suffix.toUtf8();
    return endsWith(AStringView(utf8s.begin(), utf8s.end()));
}

template<typename T>
static AOptional<T> toNumber(AStringView str) noexcept {
    if (str.empty()) return std::nullopt;
    T value = 0;
    T prevValue = 0;
    bool negative = false;

    if constexpr (std::is_integral_v<T>) {
        if (str.startsWith("0x") || str.startsWith("0X")) {
            // hex
            for (auto c : str.substr(2)) {
                value *= 16;
                if (value < prevValue) { // overflow check
                    return std::nullopt;
                }
                prevValue = value;
                if (c >= '0' && c <= '9') {
                    value += c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    value += (c - 'a') + 10;
                } else if (c >= 'A' && c <= 'F') {
                    value += (c - 'A') + 10;
                } else {
                    return std::nullopt;
                }
            }
        } else {
            auto i = str.begin();
            if (*i == '-') {
                negative = true;
                ++i;
            }
            for (; i != str.end(); ++i) {
                value *= 10;
                if (value < prevValue) { // overflow check
                    return std::nullopt;
                }
                prevValue = value;
                auto c = *i;
                if (c >= '0' && c <= '9') {
                    value += c - '0';
                } else {
                    return std::nullopt;
                }
            }
        }
    } else if constexpr (std::is_floating_point_v<T>) {
        bool fractionalPart = false;
        double fractionalPower = 0.1;

        auto i = str.begin();
        if (*i == '-') {
            negative = true;
            ++i;
        }
        for (; i != str.end(); ++i) {
            auto c = *i;
            if (c >= '0' && c <= '9') {
                T digitValue = c - '0';
                if (fractionalPart) {
                    value += digitValue * fractionalPower;
                    fractionalPower *= 0.1;
                } else {
                    value *= 10;
                    value += digitValue;
                }
            } else if (c == '.') {
                if (fractionalPart) {
                    return std::nullopt;
                }
                fractionalPart = true;
            } else {
                return std::nullopt;
            }
        }
    }

    return negative ? -value : value;
}

AOptional<int32_t> AString::toInt() const noexcept {
    return ::toNumber<int32_t>(*this);
}

AOptional<int64_t> AString::toLong() const noexcept {
    return ::toNumber<int64_t>(*this);
}

AOptional<uint32_t> AString::toUInt() const noexcept {
    return ::toNumber<uint32_t>(*this);
}

AOptional<uint64_t> AString::toULong() const noexcept {
    return ::toNumber<uint64_t>(*this);
}

AOptional<float> AString::toFloat() const noexcept {
    return ::toNumber<float>(*this);
}

AOptional<double> AString::toDouble() const noexcept {
    return ::toNumber<double>(*this);
}

AOptional<int> AString::toNumber(aui::ranged_number<int, 2, 36> base) const noexcept {
    int result = 0;
    const auto NUMBER_LAST = std::min(int('0' + int(base) - 1), int('9'));
    const auto LETTER_LAST = 'a' + int(base) - 11;
    const auto LETTER_LAST_CAPITAL = 'A' + int(base) - 11;
    for (auto c : *this) {
        if (c >= '0' && c <= NUMBER_LAST) {
            result = result * base + (c - '0');
            continue;
        }

        if (int(base) > 10) {
            if (c >= 'a' && c <= LETTER_LAST) {
                result = result * base + (c - 'a' + 10);
                continue;
            }

            if (c >= 'A' && c <= LETTER_LAST_CAPITAL) {
                result = result * base + (c - 'A' + 10);
                continue;
            }
        }
        return std::nullopt;
    }

    return result;
}

auto AString::erase(const_iterator it) -> iterator {
    if (it == cend()) {
        return end();
    }

    size_type byte_pos = it.getBytePos();

    size_type temp_pos = byte_pos;
    aui::detail::decodeUtf8At(data(), temp_pos, size());
    size_type char_byte_length = temp_pos - byte_pos;

    super::erase(byte_pos, char_byte_length);

    return iterator(this, byte_pos);
}

auto AString::erase(const_iterator begin, const_iterator end) -> iterator {
    if (begin == cend() || begin == end) {
        return iterator(this, begin == cend() ? size() : begin.getBytePos());
    }

    if (end == cend()) {
        end = cend();
    }

    size_type begin_byte_pos = begin.getBytePos();
    size_type end_byte_pos = end.getBytePos();

    if (begin_byte_pos >= end_byte_pos) {
        return iterator(this, begin_byte_pos);
    }

    size_type bytes_to_erase = end_byte_pos - begin_byte_pos;

    super::erase(begin_byte_pos, bytes_to_erase);

    return iterator(this, begin_byte_pos);
}

void AString::erase(size_t u_pos, size_t u_count) {
    erase(begin() + u_pos, begin() + u_pos + u_count);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)
