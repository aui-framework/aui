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

#include "AChar.h"

#include <AUI/Common/AStaticVector.h>

static char toByte(uint32_t value) {
    return static_cast<char>(static_cast<unsigned char>(value));
}

AStaticVector<char, 4> AChar::toUtf8() const noexcept {
    if (mValue <= 0x7F) {
        return { toByte(mValue) };
    }
    if (mValue <= 0x7FF) {
        return { toByte(0xC0 | (mValue >> 6)), toByte(0x80 | (mValue & 0x3F)) };
    }
    if (mValue <= 0xFFFF) {
        if (mValue >= 0xD800 && mValue <= 0xDFFF) {
            return {};
        }
        return {
            toByte(0xE0 | (mValue >> 12)),
            toByte(0x80 | ((mValue >> 6) & 0x3F)),
            toByte(0x80 | (mValue & 0x3F))
        };
    }
    if (mValue <= 0x10FFFF) {
        return {
            toByte(0xF0 | (mValue >> 18)),
            toByte(0x80 | ((mValue >> 12) & 0x3F)),
            toByte(0x80 | ((mValue >> 6) & 0x3F)),
            toByte(0x80 | (mValue & 0x3F))
        };
    }
    return {}; // Invalid Unicode code point
}
