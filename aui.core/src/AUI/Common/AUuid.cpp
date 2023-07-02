// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.


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

