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
    return aui::utf8::detail::decodeUtf8At(string_->data(), temp_pos, string_->size());
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
        aui::utf8::detail::decodeUtf8At(string_->data(), temp_pos, string_->size());
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
        byte_pos_ = aui::utf8::detail::getPrevCharStart(string_->data(), byte_pos_);
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

auto AUtf8MutableIterator::operator-(const AUtf8MutableIterator& other) const noexcept -> difference_type {
    return (operator AUtf8ConstIterator()) - (other.operator AUtf8ConstIterator());
}

AString AString::numberHex(int i) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%x", static_cast<unsigned>(i));
    return buf;
}

AString AString::fromUtf8(std::string_view buffer) {
    return AString(buffer);
}

AString AString::fromUtf8(AByteBufferView buffer) {
    return fromUtf8(std::string_view{buffer.data(), buffer.size()});
}

AString AString::fromUtf8(const char* str) {
    return fromUtf8(std::string_view{str, strLength(str)});
}

AString AString::fromUtf16(std::u16string_view buffer) {
    AString result;
    size_t size = simdutf::utf8_length_from_utf16(buffer.data(), buffer.size());
    result.resize(size);
    result.resize(simdutf::convert_utf16_to_utf8(buffer.data(), buffer.size(), result.data()));
    return result;
}

AString AString::fromUtf32(std::u32string_view buffer) {
    AString result;
    size_t size = simdutf::utf8_length_from_utf32(buffer.data(), buffer.size());
    result.resize(size);
    result.resize(simdutf::convert_utf32_to_utf8(buffer.data(), buffer.size(), result.data()));
    return result;
}

AString AString::fromLatin1(std::string_view buffer) {
    AString result;
    size_t size = simdutf::utf8_length_from_latin1(buffer.data(), buffer.size());
    result.resize(size);
    result.resize(simdutf::convert_latin1_to_utf8(buffer.data(), buffer.size(), result.data()));
    return result;
}

AString AString::fromLatin1(const char* str) {
    return fromLatin1({str, strLength(str)});
}

AString::AString() {}

AString::AString(const AString& other) : super(other.bytes()) {}

AString::AString(AString&& other) noexcept : super(std::move(other.bytes())) {
    other.clear(); // Windows moment
}

AString::AString(AByteBufferView buffer, AStringEncoding encoding) {
    const char* bytes = buffer.data();
    size_t size_bytes = buffer.size();
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

AString::AString(std::span<const std::byte> bytes, AStringEncoding encoding) : AString(AByteBufferView(reinterpret_cast<const char*>(bytes.data()), bytes.size()), encoding) {}

AString::AString(super::const_iterator begin, super::const_iterator end) : super(begin, end) {}

AString::AString(const_iterator begin, const_iterator end) : super(begin, end) {}

AString::AString(const char* utf8_bytes, size_type length) {
    if (simdutf::validate_utf8(utf8_bytes, length)) {
        *this = std::string(utf8_bytes, length);
    } else {
        *this = AString(length, AChar(AChar::INVALID_CHAR));
    }
}

AString::AString(const char* begin, const char* end) : super(begin, end) {}

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

AString::AString(AStringView view) : super(static_cast<std::string_view>(view.bytes())) {}

AString::AString(std::string_view view) : super(view) {}

AString::AString(const super& other) : super(other) {}

AString::AString(super&& other) : super(std::move(other)) {}

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
    bytes().insert(bytes().begin() + aui::utf8::detail::findUnicodePos(bytes(), pos).valueOr(0), utf8c.begin(), utf8c.end());
}

void AString::insert(size_type pos, AStringView str) {
    bytes().insert(bytes().begin() + aui::utf8::detail::findUnicodePos(bytes(), pos).valueOr(0), str.begin(), str.end());
}

AByteBuffer AString::encode(AStringEncoding encoding) const {
    return view().encode(encoding);
}

AString::operator AStringView() const noexcept {
    return {bytes().data(), bytes().size()};
}

AString AString::restrictLength(size_t s, const AString& stringAtEnd) const {
    if (length() > s) {
        return substr(0, s) + stringAtEnd;
    }
    return *this;
}

AString AString::trimDoubleSpace() const noexcept {
    AString s;
    s.reserve(sizeBytes());
    bool prevWasSpace = false;
    for (auto c : *this) {
        if (c == U' ') {
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
}

AString& AString::append(char c) {
    super::append(&c, 1);
    return *this;
}

AString& AString::append(AChar c) {
    auto utf8c = c.toUtf8();
    super::append(utf8c.begin(), utf8c.end());
    return *this;
}


AString& AString::replaceAll(char from, char to) {
    if (empty()) return *this;
    for (auto& i : bytes()) {
        if (i == from) {
            i = to;
        }
    }
    return *this;
}

AString& AString::replaceAll(AStringView from, AStringView to) {
    if (empty()) return *this;
    for (size_type next = 0;;)
    {
        next = find(from.bytes(), next);
        if (next == NPOS)
        {
            return *this;
        }

        auto fromLength = from.sizeBytes();
        auto toLength = to.sizeBytes();

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
        auto next = find(from.bytes(), pos);
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

bool AString::startsWith(AChar prefix) const noexcept {
    auto utf8p = prefix.toUtf8();
    return startsWith(AStringView(utf8p.data(), utf8p.size()));
}

bool AString::endsWith(AChar suffix) const noexcept {
    auto utf8s = suffix.toUtf8();
    return endsWith(AStringView(utf8s.data(), utf8s.size()));
}

auto AString::erase(const_iterator it) -> iterator {
    if (it == cend()) {
        return end();
    }

    size_type byte_pos = it.getBytePos();

    size_type temp_pos = byte_pos;
    aui::utf8::detail::decodeUtf8At(data(), temp_pos, size());
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

AStringVector AString::split(AChar c) const {
    return view().split(c);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)
