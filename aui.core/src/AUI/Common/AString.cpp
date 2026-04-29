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

void AString::insert(size_type pos, AStringView str) {
    bytes().insert(bytes().begin() + aui::utf8::detail::findUnicodePos(bytes(), pos).valueOr(0), str.begin(), str.end());
}

void AString::insert(size_type pos, AChar c) {
    auto utf8c = c.toUtf8();
    insert(pos, AStringView(utf8c.begin(), utf8c.size()));
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
    super::append(utf8c.data(), utf8c.size());
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

AStringVector AString::split(AStringView separator) const {
    return view().split(separator);
}

AStringVector AString::split(AChar separator) const {
    return view().split(separator);
}
