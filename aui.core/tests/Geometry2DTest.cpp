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

#include <gtest/gtest.h>
#include "AUI/Geometry2D/ARect.h"

TEST(Geometry2D, RectIntersect) {
    //  r10                  r100
    // +---+-------------------------------------------+
    // |   |                                           |  outside
    // +---+    r2030                                  |  +---+
    // |       +------+                                |  |   |
    // |       |      |                                |  +---+
    // |       |      |                                |
    // |       +------+                                |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // |                                               |
    // +-----------------------------------------------+
    ARect r100(glm::ivec2(0), glm::ivec2(100));
    ARect r10(glm::ivec2(0), glm::ivec2(10));
    ARect r2030(glm::ivec2(20), glm::ivec2(30));
    ARect outside(glm::ivec2(110), glm::ivec2(115));

    // inside
    EXPECT_TRUE(r100.isIntersects(r2030));
    EXPECT_TRUE(r2030.isIntersects(r100));

    EXPECT_TRUE(r100.isIntersects(r10));
    EXPECT_TRUE(r10.isIntersects(r100));

    // outside
    EXPECT_FALSE(r100.isIntersects(outside));
    EXPECT_FALSE(outside.isIntersects(r100));
}