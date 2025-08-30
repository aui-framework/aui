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

// utf8 stuff has a lot of magic
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)

inline static AStaticVector<char16_t, 4> toUtf16(char32_t i) {
    if (i <= 0xffff) {
        return { char16_t(i) };
    }

    i -= 0x10000;
    return { char16_t((i >> 10) + 0xD800),
             char16_t((i & 0x3FF) + 0xDC00) };
}

template<typename T>
inline static char32_t fromUtf16(T& iterator, T last) {
    auto c1 = *(iterator++);
    if (iterator == last) {
        // incomplete sequence?
        return static_cast<char32_t >(c1);
    }
    if (*iterator < 0xD800) {
        return static_cast<char32_t >(c1);
    }
    auto c2 = *(iterator++);

    if (c2 < 0xDC00) {
        // bad entity
    }

    c1 -= 0xD800;
    c2 -= 0xDC00;

    return (char32_t(c1) << 10 | char32_t(c2 & 0x3FF)) + 0x10000;
}

inline static void fromUtf8_impl(AString& destination, const char* str, size_t length) {
    destination.reserve(length);

    // parse utf8
    for (; length && *str; --length)
    {
        if ((*str & 0b1000'0000) == 0) {
            // ascii symbol
            destination.push_back(*(str++));
            continue;
        }
        // utf8 symbol

        if ((*str & 0b1110'0000) == 0b1100'0000) {
            // 2-byte symbol
            char16_t t = *(str++) & 0b11111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            destination.push_back(t);
            length -= 1;
            continue;
        }

        if ((*str & 0b1111'0000) == 0b1110'0000) {
            // 3-byte symbol
            char16_t t = *(str++) & 0b1111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            destination.push_back(t);
            length -= 2;
            continue;
        }

        if ((*str & 0b1111'1000) == 0b1111'0000) {
            // 4-byte symbol
            char32_t t = *(str++) & 0b111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            t <<= 6;
            t |= *(str++) & 0b111111;
            destination.insertAll(toUtf16(t));
            length -= 3;
            continue;
        }


        str++; // bad entity?
    }
}

AString::AString(const char* utf8) noexcept
{
    fromUtf8_impl(*this, utf8, std::strlen(utf8));
}

AString::AString(std::string_view utf8) noexcept
{
    fromUtf8_impl(*this, utf8.data(), utf8.length());
}

AString::AString(const std::string& utf8) noexcept
{
    fromUtf8_impl(*this, utf8.c_str(), utf8.length());
}

AString AString::fromUtf8(const AByteBufferView& buffer) {
    return AString::fromUtf8(buffer.data(), buffer.size());
}

AString AString::fromUtf8(const char* buffer, size_t length) {
    AString result;
    fromUtf8_impl(result, buffer, length);
    return result;
}


AByteBuffer AString::toUtf8() const noexcept
{
    AByteBuffer buf;
    for (auto it = begin(); it != end();)
    {
        auto c = *it;
        if (c < 0x80) {
            buf << static_cast<char>(c);
            ++it;
            continue;
        }

        if (c < 0x800) {
            char b[] = {
                static_cast<char>(0b11000000 | (c >> 6 & 0b11111)),
                static_cast<char>(0b10000000 | (c      & 0b111111)),
                0,
            };
            buf << b;
            ++it;
            continue;
        }

        if (c < 0xD800) {
            char b[] = {
                static_cast<char>(0b11100000 | (c >> 12 & 0b1111)),
                static_cast<char>(0b10000000 | (c >> 6  & 0b111111)),
                static_cast<char>(0b10000000 | (c       & 0b111111)),
                0,
            };
            buf << b;
            ++it;
            continue;
        }

        {
            const auto c = fromUtf16(it, end());

            char b[] = {
                    static_cast<char>(0b11110000 | (c >> 18 & 0b111)),
                    static_cast<char>(0b10000000 | (c >> 12 & 0b111111)),
                    static_cast<char>(0b10000000 | (c >> 6 & 0b111111)),
                    static_cast<char>(0b10000000 | (c & 0b111111)),
                    0,
            };
            buf << b;
        }
    }
    return buf;
}

AStringVector AString::split(char16_t c) const noexcept
{
    if (empty()) {
        return {};
    }
    AStringVector result;
    result.reserve(length() / 10);
    for (size_type s = 0;;)
    {
        auto next = super::find(c, s);
        if (next == npos)
        {
            result << substr(s);
            break;
        }

        result << substr(s, next - s);
        s = next + 1;
    }
    return result;
}

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

AString AString::fromLatin1(const AByteBuffer& buffer)
{
    return {buffer.begin(), buffer.end() };
}


AString AString::fromLatin1(const char* buffer) {
    AString s;
    for (; *buffer; ++buffer)
        s.push_back(*buffer);

    return s;
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

std::string AString::toStdString() const noexcept
{
    auto encoded = toUtf8();
    std::string dst;
    dst.reserve(encoded.getSize());
    dst.insert(0, encoded.data(), encoded.getSize());

    return dst;
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

void AString::resizeToNullTerminator() {
    char16_t* i = data();
    for (; *i; ++i);
    resize(i - data());
}

AString AString::restrictLength(size_t s, const AString& stringAtEnd) const {
    if (length() > s) {
        return substr(0, s) + stringAtEnd;
    }
    return *this;
}

AString AString::numberHex(int i) noexcept {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%x", static_cast<unsigned>(i));
    return buf;
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
}

template<typename T>
AOptional<T> AString::toNumberImpl() const noexcept {
    if (empty()) return std::nullopt;
    T value = 0;
    T prevValue = 0;
    bool negative = false;

    if constexpr (std::is_integral_v<T>) {
        if (startsWith("0x") || startsWith("0X")) {
            // hex
            for (auto c : substr(2)) {
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
            auto i = begin();
            if (*i == '-') {
                negative = true;
                ++i;
            }
            for (; i != end(); ++i) {
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

        auto i = begin();
        if (*i == '-') {
            negative = true;
            ++i;
        }
        for (; i != end(); ++i) {
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

AOptional<int> AString::toInt() const noexcept {
    return toNumberImpl<int>();
}

AOptional<int64_t> AString::toLongInt() const noexcept {
    return toNumberImpl<int64_t>();
}

AOptional<unsigned> AString::toUInt() const noexcept {
    return toNumberImpl<unsigned>();
}

AOptional<double> AString::toDouble() const noexcept {
    return toNumberImpl<double>();
}

AOptional<float> AString::toFloat() const noexcept {
    return toNumberImpl<float>();
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-bounds-pointer-arithmetic)
