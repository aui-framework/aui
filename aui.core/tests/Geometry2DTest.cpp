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

#include <gtest/gtest.h>
#include "AUI/Geometry2D/ARect.h"

TEST(Geometry2D, RectIntersect1) {
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
    ARect<int> r100{glm::ivec2(0), glm::ivec2(100)};
    ARect<int> r10{glm::ivec2(0), glm::ivec2(10)};
    ARect<int> r2030{glm::ivec2(20), glm::ivec2(30)};
    ARect<int> outside{glm::ivec2(110), glm::ivec2(115)};

    // inside
    EXPECT_TRUE(r100.isIntersects(r2030));
    EXPECT_TRUE(r2030.isIntersects(r100));

    EXPECT_TRUE(r100.isIntersects(r10));
    EXPECT_TRUE(r10.isIntersects(r100));

    // outside
    EXPECT_FALSE(r100.isIntersects(outside));
    EXPECT_FALSE(outside.isIntersects(r100));
}

TEST(Geometry2D, RectIntersect2) {
    ARect<int> r1{glm::ivec2(0), glm::ivec2(84, 54)};
    ARect<int> r2{glm::ivec2(7, -8), glm::ivec2(77, 62)};
    EXPECT_TRUE(r1.isIntersects(r2));
    EXPECT_TRUE(r2.isIntersects(r1));
}
