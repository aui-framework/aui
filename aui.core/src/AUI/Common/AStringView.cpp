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

#include <AUI/Common/AByteBuffer.h>
#include <simdutf.h>

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

#if AUI_PLATFORM_WIN
std::wstring AStringView::toWideString() const {
    static_assert(sizeof(wchar_t) == sizeof(char16_t), "This method is only for 16-bit wchar");
    size_t words = simdutf::utf16_length_from_utf8(super::data(), super::size());
    std::wstring encoded(words, L'\0');
    auto size = simdutf::convert_utf8_to_utf16(super::data(), super::size(), reinterpret_cast<char16_t*>(encoded.data()));
    encoded[words] = '\0';
    encoded.resize(size);
    return std::move(encoded);
}
#endif

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
