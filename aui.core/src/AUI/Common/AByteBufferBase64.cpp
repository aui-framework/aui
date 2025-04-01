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
// Created by Alex2772 on 7/13/2022.
//

#include <AUI/Common/AByteBuffer.h>
#include <AUI/Common/AByteBufferView.h>

static const std::string_view BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";



AString AByteBufferView::toBase64String() const {
    size_t resultLength = 4 * ((size() + 2) / 3);
    AString result(resultLength, '\0');
    size_t i;
    auto p = const_cast<char16_t*>(result.c_str());

    for (i = 0; i < size() - 2; i += 3) {
        *p++ = BASE64_CHARS[(data()[i] >> 2) & 0x3F];
        *p++ = BASE64_CHARS[((data()[i] & 0x3) << 4) | ((int) (data()[i + 1] & 0xF0) >> 4)];
        *p++ = BASE64_CHARS[((data()[i + 1] & 0xF) << 2) | ((int) (data()[i + 2] & 0xC0) >> 6)];
        *p++ = BASE64_CHARS[data()[i + 2] & 0x3F];
    }
    if (i < size()) {
        *p++ = BASE64_CHARS[(data()[i] >> 2) & 0x3F];
        if (i == (size() - 1)) {
            *p++ = BASE64_CHARS[((data()[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = BASE64_CHARS[((data()[i] & 0x3) << 4) | ((int) (data()[i + 1] & 0xF0) >> 4)];
            *p++ = BASE64_CHARS[((data()[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    return result;
}



static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

AByteBuffer AByteBuffer::fromBase64String(const AString& encodedString) {
    int in_len = encodedString.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    AByteBuffer ret;

    while (in_len-- && ( encodedString[in_] != '=') && is_base64(encodedString[in_])) {
        char_array_4[i++] = encodedString[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = BASE64_CHARS.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret << char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = BASE64_CHARS.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret << char_array_3[j];
    }

    return ret;
}
