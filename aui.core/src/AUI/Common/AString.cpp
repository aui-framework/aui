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

static AStaticVector<char, 4> toUtf8(char32_t i) noexcept {
    if (i <= 0x7F) {
        return { static_cast<char>(i) };
    }
    if (i <= 0x7FF) {
        return { static_cast<char>(0xC0 | (i >> 6)), static_cast<char>(0x80 | (i & 0x3F)) };
    }
    if (i <= 0xFFFF) {
        if (i >= 0xD800 && i <= 0xDFFF) {
            return {}; // Invalid Unicode code point
        }
        return {
            static_cast<char>(0xE0 | (i >> 12)),
            static_cast<char>(0x80 | ((i >> 6) & 0x3F)),
            static_cast<char>(0x80 | (i & 0x3F))
        };
    }
    if (i <= 0x10FFFF) {
        return {
            static_cast<char>(0xF0 | (i >> 18)),
            static_cast<char>(0x80 | ((i >> 12) & 0x3F)),
            static_cast<char>(0x80 | ((i >> 6) & 0x3F)),
            static_cast<char>(0x80 | (i & 0x3F))
        };
    }
    return {}; // Invalid Unicode code point
}

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

}

AString AString::numberHex(int i) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%x", static_cast<unsigned>(i));
    return buf;
}

AString::AString(std::span<const std::byte> bytes, AStringEncoding encoding) {
    switch (encoding) {
        case AStringEncoding::UTF8: {
            super::resize(bytes.size());
            std::memcpy(data(), bytes.data(), bytes.size());
        } break;
        case AStringEncoding::UTF16: {
            size_t size = simdutf::utf8_length_from_utf16(reinterpret_cast<const char16_t*>(bytes.data()), bytes.size() / 2);
            super::resize(size);
            super::resize(simdutf::convert_utf16_to_utf8(reinterpret_cast<const char16_t*>(bytes.data()), bytes.size() / 2, data()));
        } break;
        case AStringEncoding::UTF32: {
            size_t size = simdutf::utf8_length_from_utf32(reinterpret_cast<const char32_t*>(bytes.data()), bytes.size() / 4);
            super::resize(size);
            super::resize(simdutf::convert_utf32_to_utf8(reinterpret_cast<const char32_t*>(bytes.data()), bytes.size() / 4, data()));
        } break;
        case AStringEncoding::LATIN1: {
            size_t size = simdutf::utf8_length_from_latin1(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            super::resize(size);
            super::resize(simdutf::convert_latin1_to_utf8(reinterpret_cast<const char*>(bytes.data()), bytes.size(), data()));
        } break;
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

void AString::push_back(AChar c) noexcept {
    append(c);
}

AByteBuffer AString::encode(AStringEncoding encoding) const {
    AByteBuffer bytes;
    if (super::empty()) return bytes;
    switch (encoding) {
        case AStringEncoding::UTF8: {
            bytes.reserve(super::size());
            bytes.write(super::data(), super::size());
        } break;
        case AStringEncoding::UTF16: {
            size_t words = simdutf::utf16_length_from_utf8(super::data(), super::size());
            bytes.resize(words * 2);
            bytes.resize(simdutf::convert_utf8_to_utf16(super::data(), super::size(), reinterpret_cast<char16_t*>(bytes.data())));
        } break;
        case AStringEncoding::UTF32: {
            size_t words = simdutf::utf32_length_from_utf8(super::data(), super::size());
            bytes.resize(words * 4);
            bytes.resize(simdutf::convert_utf8_to_utf32(super::data(), super::size(), reinterpret_cast<char32_t*>(bytes.data())));
        } break;
        case AStringEncoding::LATIN1: {
            size_t words = simdutf::latin1_length_from_utf8(super::data(), super::size());
            bytes.resize(words);
            bytes.resize(simdutf::convert_utf8_to_latin1(super::data(), super::size(), reinterpret_cast<char*>(bytes.data())));
        } break;
    }
    return std::move(bytes);
}

/*
AString AString::trimLeft(char16_t symbol) const noexcept
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

AString AString::trimRight(char16_t symbol) const noexcept
{
    for (auto i = rbegin(); i != rend(); ++i)
    {
        if (*i != symbol)
        {
            return { begin(),i.base() };
        }
    }
    return {};
}

AString& AString::replaceAll(char16_t from, char16_t to) noexcept {
    for (auto& s : *this) {
        if (s == from)
            s = to;
    }
    return *this;
}

AString& AString::replaceAll(const AString& from, const AString& to) {
    for (size_type next = 0;;)
    {
        next = find(from, next);
        if (next == NPOS)
        {
            return *this;
        }

        auto fromLength = from.length();
        auto toLength = to.length();

        if (fromLength == toLength) {
            for (auto c : to) {
                *(begin() + next++) = c;
            }
        } else if (fromLength < toLength) {
            const auto diff = toLength - fromLength;
            for (auto c : aui::range(to.begin(), to.end() - diff)) {
                *(begin() + next++) = c;
            }
            next = std::distance(begin(), insert(begin() + next, to.begin() + fromLength, to.end())) + 1;
        } else {
            for (auto c : to) {
                *(begin() + next++) = c;
            }
            const auto diff = fromLength - toLength;
            erase(begin() + next, begin() + next + diff);
        }
    }
    return *this;
}

AString AString::replacedAll(const AString& from, const AString& to) const
{
    AString result;

    result.reserve(size());

    for (size_type pos = 0;;)
    {
        auto next = find(from, pos);
        if (next == NPOS)
        {
            result.insert(result.end(), begin() + pos, end());
            return result;
        }

        result.insert(result.end(), begin() + pos, begin() + next);
        result.insert(result.end(), to.begin(), to.end());

        pos = next + from.length();
    }

    return result;
}

void AString::resizeToNullTerminator() {
    const char* end = data();
    while (*end) {
        ++end;
    }
    resize(end - data());
}*/

AString::operator AStringView() const noexcept {
    return {static_cast<const std::string&>(*this)};
}

AString::size_type AString::length() const noexcept {
    return simdutf::count_utf8(super::data(), super::size());
}

AString AString::trimLeft(char symbol) const
{
    for (auto i = byte_begin(); i != byte_end(); ++i)
    {
        if (*i != symbol)
        {
            return { i, byte_end() };
        }
    }
    return {};
}

AString AString::trimRight(char symbol) const
{
    for (auto i = byte_rbegin(); i != byte_rend(); ++i)
    {
        if (*i != symbol)
        {
            return { byte_begin(), i.base() };
        }
    }
    return {};
}

AString AString::trim(char symbol) const
{
    auto left = byte_begin();
    auto right = byte_end();

    while (left != right && *left == symbol)
    {
        ++left;
    }

    if (left != right)
    {
        auto riter = byte_rbegin();
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

AString AString::processEscapes() const {
    AString result;
    result.reserve(length());
    bool doEscape = false;
    for (auto& c : *this) {
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
    auto utf8c = ::toUtf8(c);
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
    auto utf8c = ::toUtf8(c);
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

template<typename T>
static AOptional<T> toNumber(const AStringView& str) noexcept {
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
    return toNumber<int32_t>(*this);
}

AOptional<int64_t> AString::toLong() const noexcept {
    return toNumber<int64_t>(*this);
}

AOptional<uint32_t> AString::toUInt() const noexcept {
    return toNumber<uint32_t>(*this);
}

AOptional<uint64_t> AString::toULong() const noexcept {
    return toNumber<uint64_t>(*this);
}

AOptional<float> AString::toFloat() const noexcept {
    return toNumber<float>(*this);
}

AOptional<double> AString::toDouble() const noexcept {
    return toNumber<double>(*this);
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

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)
