/**
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


//
// Created by alex2772 on 4/20/21.
//

#include <AUI/Traits/memory.h>
#include "AUuid.h"

AUuid::AUuid(const AString& s) {
    size_t dataIndex = 0;
    for (size_t stringIndex = 0; stringIndex < s.length();) {
        if (stringIndex + 1 >= s.length()) {
            throw AUuidException("too short uuid string: " + s);
        }
        auto b1 = s[stringIndex];
        auto b2 = s[stringIndex + 1];
        if (b1 == '-') {
            ++stringIndex;
        } else if (b2 == '-') {
            throw AUuidException("invalid uuid dash format: " + s);
        } else {
            mData[dataIndex++] = fromHex(b1) << 4u | fromHex(b2);
            stringIndex += 2;
        }
    }
    if (dataIndex != mData.size()) {
        throw AUuidException("too short uuid string: " + s);
    }
}

uint8_t AUuid::fromHex(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    throw AUuidException("invalid hex char in uuid: "_as + c);
}

AString AUuid::toString() const {
    char str[40];
    sprintf(str,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            mData[0],
            mData[1],
            mData[2],
            mData[3],
            mData[4],
            mData[5],
            mData[6],
            mData[7],
            mData[8],
            mData[9],
            mData[10],
            mData[11],
            mData[12],
            mData[13],
            mData[14],
            mData[15]
            );
    return AString(str, str + 36);
}

AString AUuid::toRawString() const {
    char str[40];
    sprintf(str,
            "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            mData[0],
            mData[1],
            mData[2],
            mData[3],
            mData[4],
            mData[5],
            mData[6],
            mData[7],
            mData[8],
            mData[9],
            mData[10],
            mData[11],
            mData[12],
            mData[13],
            mData[14],
            mData[15]
    );
    return AString(str, str + 32);
}

AUuid AUuid::fromString(const AString& string) {
    try {
        return AUuid{string};
    } catch (const AUuidException&) {
        std::array<uint8_t, 16> v;
        aui::zero(v);
        size_t i = 0;
        for (auto c : string) {
            while (c >= 255) {
                v[i++] = c;
                c >>= 8;
            }
            v[i++] = c;
        }
        return AUuid{v};
    }
}

