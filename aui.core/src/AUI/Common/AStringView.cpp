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

#include "AStringView.h"
#include "AStaticVector.h"

#include <AUI/Common/AByteBuffer.h>
#include <simdutf.h>

bool AStringView::contains(char c) const noexcept {
    return contains(AChar(c));
}

bool AStringView::contains(AChar c) const noexcept {
    for (auto it = begin(); it != end(); ++it) {
        if (*it == c) {
            return true;
        }
    }
    return false;
}

bool AStringView::contains(AStringView str) const noexcept {
    if (str.empty()) return true;
    if (str.length() > length()) return false;
    return std::search(bytes().begin(), bytes().end(), str.bytes().begin(), str.bytes().end()) != bytes().end();
}

AByteBuffer AStringView::encode(AStringEncoding encoding) const {
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
            bytes.reserve((words + 1) * sizeof(char16_t));
            bytes.resize(simdutf::convert_utf8_to_utf16(super::data(), super::size(), reinterpret_cast<char16_t*>(bytes.data())) * sizeof(char16_t));
            reinterpret_cast<char16_t*>(bytes.data())[words] = '\0';
        } break;
        case AStringEncoding::UTF32: {
            size_t words = simdutf::utf32_length_from_utf8(super::data(), super::size());
            bytes.reserve((words + 1) * sizeof(char32_t));
            bytes.resize(simdutf::convert_utf8_to_utf32(super::data(), super::size(), reinterpret_cast<char32_t*>(bytes.data())) * sizeof(char32_t));
            reinterpret_cast<char32_t*>(bytes.data())[words] = '\0';
        } break;
        case AStringEncoding::LATIN1: {
            size_t words = simdutf::latin1_length_from_utf8(super::data(), super::size());
            bytes.reserve(words + 1);
            bytes.resize(simdutf::convert_utf8_to_latin1(super::data(), super::size(), reinterpret_cast<char*>(bytes.data())));
            bytes.data()[bytes.capacity() - 1] = '\0';
        } break;
    }
    return std::move(bytes);
}

std::u16string AStringView::toUtf16() const {
    size_t words = simdutf::utf16_length_from_utf8(super::data(), super::size());
    std::u16string encoded(words, '\0');
    auto size = simdutf::convert_utf8_to_utf16(super::data(), super::size(), encoded.data());
    encoded[words] = '\0';
    encoded.resize(size);
    return std::move(encoded);
}

std::u32string AStringView::toUtf32() const {
    size_t words = simdutf::utf16_length_from_utf8(super::data(), super::size());
    std::u32string encoded(words, U'\0');
    auto size = simdutf::convert_utf8_to_utf32(super::data(), super::size(), encoded.data());
    encoded[words] = '\0';
    encoded.resize(size);
    return std::move(encoded);
}

AStringView::size_type AStringView::length() const noexcept {
    return simdutf::count_utf8(super::data(), super::size());
}

AStringView
AStringView::substr(AStringView::size_type pos, AStringView::size_type count) const noexcept {
    auto it = begin();
    for (; it != end() && pos > 0; ++it, --pos);
    auto begin = it;
    for (; it != end() && count > 0; ++it, --count);
    return AStringView(begin.data(), it.data() - begin.data());
}

AString AStringView::uppercase() const {
    std::string buf{bytes()};
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

AString AStringView::lowercase() const {
    std::string buf{bytes()};
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

template<typename T>
static AOptional<T> toNumber(std::string_view str) noexcept {
    if (str.empty()) return std::nullopt;
    T value = 0;
    T prevValue = 0;
    bool negative = false;

    if constexpr (std::is_integral_v<T>) {
        if (str.starts_with("0x") || str.starts_with("0X")) {
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
            if (i == str.end()) return std::nullopt;
            for (; i != str.end(); ++i) {
                auto c = *i;
                if (c >= '0' && c <= '9') {
                    value *= 10;
                    if (value < prevValue) { // overflow check
                        return std::nullopt;
                    }
                    prevValue = value;
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

AOptional<int32_t> AStringView::toInt() const noexcept {
    return ::toNumber<int32_t>(bytes());
}

AOptional<int64_t> AStringView::toLong() const noexcept {
    return ::toNumber<int64_t>(bytes());
}

AOptional<uint32_t> AStringView::toUInt() const noexcept {
    return ::toNumber<uint32_t>(bytes());
}

AOptional<uint64_t> AStringView::toULong() const noexcept {
    return ::toNumber<uint64_t>(bytes());
}

AOptional<float> AStringView::toFloat() const noexcept {
    return ::toNumber<float>(bytes());
}

AOptional<double> AStringView::toDouble() const noexcept {
    return ::toNumber<double>(*this);
}


AOptional<int> AStringView::toNumber(aui::ranged_number<int, 2, 36> base) const noexcept {
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

bool AStringView::toBool() const {
    return sizeBytes() == 4 && lowercase() == "true";
}

AStringView AStringView::trimLeft(AChar symbol) const {
    for (auto i = begin(); i != end(); ++i)
    {
        if (*i != symbol)
        {
            return AStringView(i.data(), npos);
        }
    }
    return {};
}

AStringView AStringView::trimRight(AChar symbol) const {
    for (auto i = rbegin(); i != rend(); ++i)
    {
        if (*i != AChar(symbol))
        {
            return std::string_view(data(), i.base().data() - data());
        }
    }
    return {};
}

AStringVector AStringView::split(AChar c) const {
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

AString AStringView::removedAll(AChar c) {
    AString copy{*this};
    copy.removeAll(c);
    return copy;
}

#if AUI_PLATFORM_WIN
namespace aui::win32 {
std::wstring toWchar(AStringView str) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t size must be same as char16_t");
    size_t words = simdutf::utf16_length_from_utf8(str.data(), str.size());
    std::wstring encoded(words, L'\0');
    auto size = simdutf::convert_utf8_to_utf16(str.data(), str.size(), reinterpret_cast<char16_t*>(encoded.data()));
    encoded[words] = '\0';
    encoded.resize(size);
    return std::move(encoded);
}
}
#endif
