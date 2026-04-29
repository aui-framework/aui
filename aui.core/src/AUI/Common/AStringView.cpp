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

#include <AUI/Common/AStaticVector.h>
#include <AUI/Common/AByteBuffer.h>
#include <simdutf.h>

bool AStringView::contains(char c) const noexcept {
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

AString AStringView::uppercase() const {
    std::string buf{bytes()};
    std::transform(buf.begin(), buf.end(), buf.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return buf;
}

AString AStringView::lowercase() const {
    std::string buf{bytes()};
    std::transform(buf.begin(), buf.end(), buf.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
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
    if (sizeBytes() != 4) {
        return false;
    }
    const char* d = data();
    return (d[0] == 't' || d[0] == 'T') &&
           (d[1] == 'r' || d[1] == 'R') &&
           (d[2] == 'u' || d[2] == 'U') &&
           (d[3] == 'e' || d[3] == 'E');
}

AString AStringView::replacedAll(AStringView from, AStringView to) const {
    AString result;
    result.reserve(size());

    for (size_type pos = 0;;) {
        auto next = find(from.bytes(), pos);
        if (next == npos) {
            result.bytes().insert(result.bytes().end(),
                                  bytes().begin() + pos, bytes().end());
            return result;
        }

        result.bytes().insert(result.bytes().end(),
                              bytes().begin() + pos, bytes().begin() + next);
        result.bytes().insert(result.bytes().end(),
                              to.bytes().begin(), to.bytes().end());

        pos = next + from.length();
    }
}

AString AStringView::replacedAll(AChar from, AChar to) const {
    auto utf8from = from.toUtf8();
    auto utf8to   = to.toUtf8();
    return replacedAll(AStringView(utf8from.begin(), utf8from.end()),
                       AStringView(utf8to.begin(),   utf8to.end()));
}

AString AStringView::removedAll(AStringView seq) const {
    AString result;
    result.reserve(size());

    for (size_type pos = 0;;) {
        auto next = find(seq.bytes(), pos);
        if (next == npos) {
            result.bytes().insert(result.bytes().end(),
                                  bytes().begin() + pos, bytes().end());
            return result;
        }

        result.bytes().insert(result.bytes().end(),
                              bytes().begin() + pos, bytes().begin() + next);

        pos = next + seq.length();
    }
}

AString AStringView::removedAll(AChar c) const {
    auto utf8c = c.toUtf8();
    return removedAll(AStringView(utf8c.begin(), utf8c.end()));
}

AStringVector AStringView::split(AStringView separator) const {
    if (empty()) {
        return {};
    }

    AStringVector result;
    result.reserve(length() / 10);

    for (size_type s = 0;;) {
        auto next = find(separator, s);
        if (next == npos) {
            result << substr(s);
            break;
        }
        result << substr(s, next - s);
        s = next + separator.length();
    }

    return result;
}

AStringVector AStringView::split(AChar separator) const {
    auto utf8s = separator.toUtf8();
    return split(AStringView(utf8s.begin(), utf8s.end()));
}

#if AUI_PLATFORM_WIN
namespace aui::win32 {
std::wstring toWchar(AStringView str) {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t size must be same as char16_t");
    size_t words = simdutf::utf16_length_from_utf8(str.data(), str.sizeBytes());
    std::wstring encoded(words, L'\0');
    auto size = simdutf::convert_utf8_to_utf16(str.data(), str.sizeBytes(), reinterpret_cast<char16_t*>(encoded.data()));
    encoded[words] = '\0';
    encoded.resize(size);
    return std::move(encoded);
}
}
#endif
