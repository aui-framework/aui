/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <cstring>
#include "AString.h"
#include "AStringVector.h"
#include <AUI/Common/AByteBuffer.h>

inline static void fromUtf8_impl(AString& destination, const char* str, size_t length) {
    destination.reserve(length);

    // parse utf8
    for (; *str && length; --length)
    {
        if (*str & 0x80)
        {
            wchar_t t;
            // utf8 symbol
            if (*str & 0b00100000)
            {
                // 3-byte symbol
                t = *(str++) & 0b1111;
                t <<= 6;
                t |= *(str++) & 0b111111;
                t <<= 6;
                t |= *(str++) & 0b111111;
                destination.push_back(t);
                length -= 2;
            } else
            {
                // 2-byte symbol
                t = *(str++) & 0b11111;
                t <<= 6;
                t |= *(str++) & 0b111111;
                destination.push_back(t);
                length -= 1;
            }
        } else
        {
            // ascii symbol
            destination.push_back(*(str++));
        }
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
    for (wchar_t c : *this)
    {
        if (c >= 0x80)
        {
            if (c >= 0x800)
            {
                char b[] = {
                        static_cast<char>(0b11100000 | (c >> 12 & 0b1111)),
                        static_cast<char>(0b10000000 | (c >> 6 & 0b111111)),
                        static_cast<char>(0b10000000 | (c & 0b111111)),
                        0,
                };
                buf << b;
            } else if (c >= 0x80)
            {
                char b[] = {
                        static_cast<char>(0b11000000 | (c >> 6 & 0b11111)),
                        static_cast<char>(0b10000000 | (c & 0b111111)),
                        0,
                };
                buf << b;
            }
        } else
        {
            buf << *reinterpret_cast<char*>(&c);
        }
    }
    return buf;
}

AStringVector AString::split(wchar_t c) const noexcept
{
    if (empty()) {
        return {};
    }
    AStringVector result;
    result.reserve(length() / 10);
    for (size_type s = 0;;)
    {
        auto next = std::wstring::find(c, s);
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

AString AString::trimLeft(wchar_t symbol) const noexcept
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

AString AString::trimRight(wchar_t symbol) const noexcept
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

AString& AString::replaceAll(wchar_t from, wchar_t to) noexcept {
    for (auto& s : *this) {
        if (s == from)
            s = to;
    }
    return *this;
}

AString& AString::replaceAll(const AString& from, const AString& to) {
    for (size_type pos = 0;;)
    {
        auto next = find(from, pos);
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
            insert(begin() + next, to.begin() + fromLength, to.end());
            next += diff;
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
    result.reserve(size() * to.length() / from.length());
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


int AString::toNumberDec() const noexcept
{
    int n;
    if (std::swscanf(c_str(), L"%d", &n) < 0)
        return -1;

    return n;
}

int AString::toNumberHex() const noexcept
{
    int n;
    if (std::swscanf(c_str(), L"%x", &n) < 0)
        return -1;

    return n;
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
                                else if ((*p >= 0xb0)
                                         && ((*p <= 0xb5)
                                             || (*p == 0xb7))
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
                                if ((*p >= 0x80)
                                    && ((*p <= 0x85)
                                        || (*p == 0x87))
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
    wchar_t* i;
    for (i = data(); *i; ++i);
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
    sprintf(buf, "%x", i);
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
