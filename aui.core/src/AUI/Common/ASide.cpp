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

#include "ASide.h"

glm::vec2 ASides::getNormalVector(ASide s) {
    switch (s) {
        case ASide::TOP:
            return {0, -1};
        case ASide::BOTTOM:
            return {0, 1};
        case ASide::LEFT:
            return {-1, 0};
        case ASide::RIGHT:
            return {1, 0};
        case ASide::NONE:
            return {};
        default:
            break;
    }
    glm::ivec2 result{0};

    if (!!(s & ASide::LEFT)) {
        result += glm::vec2(-1, 0);
    }
    if (!!(s & ASide::RIGHT)) {
        result += glm::vec2(1, 0);
    }
    if (!!(s & ASide::TOP)) {
        result += glm::vec2(0, -1);
    }
    if (!!(s & ASide::BOTTOM)) {
        result += glm::vec2(0, 1);
    }

    return result;
}
