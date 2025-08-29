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
    std::snprintf(str, sizeof(str),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        static_cast<unsigned>(mData[0]),
        static_cast<unsigned>(mData[1]),
        static_cast<unsigned>(mData[2]),
        static_cast<unsigned>(mData[3]),
        static_cast<unsigned>(mData[4]),
        static_cast<unsigned>(mData[5]),
        static_cast<unsigned>(mData[6]),
        static_cast<unsigned>(mData[7]),
        static_cast<unsigned>(mData[8]),
        static_cast<unsigned>(mData[9]),
        static_cast<unsigned>(mData[10]),
        static_cast<unsigned>(mData[11]),
        static_cast<unsigned>(mData[12]),
        static_cast<unsigned>(mData[13]),
        static_cast<unsigned>(mData[14]),
        static_cast<unsigned>(mData[15])
    );
    return AString(str, str + 36);
}

AString AUuid::toRawString() const {
    char str[40];
    std::snprintf(str, sizeof(str),
        "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        static_cast<unsigned>(mData[0]),
        static_cast<unsigned>(mData[1]),
        static_cast<unsigned>(mData[2]),
        static_cast<unsigned>(mData[3]),
        static_cast<unsigned>(mData[4]),
        static_cast<unsigned>(mData[5]),
        static_cast<unsigned>(mData[6]),
        static_cast<unsigned>(mData[7]),
        static_cast<unsigned>(mData[8]),
        static_cast<unsigned>(mData[9]),
        static_cast<unsigned>(mData[10]),
        static_cast<unsigned>(mData[11]),
        static_cast<unsigned>(mData[12]),
        static_cast<unsigned>(mData[13]),
        static_cast<unsigned>(mData[14]),
        static_cast<unsigned>(mData[15])
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
        for (auto c : string.bytes()) {
            while (c >= 255) {
                v[i++] = c;
                c >>= 8;
            }
            v[i++] = c;
        }
        return AUuid{v};
    }
}

