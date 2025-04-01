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

#pragma once

#include "AUI/Reflect/AEnumerate.h"

/// [FieldCell]
AUI_ENUM_FLAG(FieldCell) {
    EMPTY = 0,
    HAS_BOMB = 1,
    OPEN = 2,
    HAS_FLAG = 4,
    RED_BG = 8,
    DONT_PLANT_BOMB_HERE = 16,
};
/// [FieldCell]

namespace field_cell {
    inline uint16_t getBombCountAround(FieldCell fc) {
        return int(fc) >> 16;
    }

    inline void setBombCountAround(FieldCell& fc, uint16_t count) {
        reinterpret_cast<std::underlying_type_t<FieldCell>&>(fc) &= 0xffff;
        reinterpret_cast<std::underlying_type_t<FieldCell>&>(fc) |= int(count) << 16;
    }
}
