/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Reflect/AEnumerate.h>

enum class ALayoutDirection {
    NONE,
    HORIZONTAL,
    VERTICAL,
};

AUI_ENUM_VALUES(ALayoutDirection,
                ALayoutDirection::NONE,
                ALayoutDirection::HORIZONTAL,
                ALayoutDirection::VERTICAL)

namespace aui::layout_direction {
template<typename T>
[[nodiscard]]
inline T& getAxisValue(ALayoutDirection direction, glm::tvec2<T>& v) {
    switch (direction) {
        case ALayoutDirection::VERTICAL  : return v.y;
        case ALayoutDirection::HORIZONTAL: return v.x;
        default: throw AException("invalid direction");
    }
}

template<typename T>
[[nodiscard]]
inline T getAxisValue(ALayoutDirection direction, const glm::tvec2<T>& v) {
    switch (direction) {
        case ALayoutDirection::VERTICAL  : return v.y;
        case ALayoutDirection::HORIZONTAL: return v.x;
        default: throw AException("invalid direction");
    }
}

template<typename T>
[[nodiscard]]
inline T& getPerpendicularAxisValue(ALayoutDirection direction, glm::tvec2<T>& v) {
    switch (direction) {
        case ALayoutDirection::VERTICAL  : return v.x;
        case ALayoutDirection::HORIZONTAL: return v.y;
        default: throw AException("invalid direction");
    }
}

template<typename T>
[[nodiscard]]
inline T getPerpendicularAxisValue(ALayoutDirection direction, const glm::tvec2<T>& v) {
    switch (direction) {
        case ALayoutDirection::VERTICAL  : return v.x;
        case ALayoutDirection::HORIZONTAL: return v.y;
        default: throw AException("invalid direction");
    }
}
}