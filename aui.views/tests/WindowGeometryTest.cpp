/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/Platform/detail/WindowGeometry.h>

using namespace aui::platform::detail;

TEST(WindowGeometry, PointsToPixelsUsesBackingScale) {
    EXPECT_EQ(pointsToPixels(800, 2.0), 1600);
    EXPECT_EQ(pointsToPixels(800, 1.0), 800);
}

TEST(WindowGeometry, PixelsToPointsIsInverse) {
    EXPECT_DOUBLE_EQ(pixelsToPoints(1600, 2.0), 800.0);
    EXPECT_EQ(pointsToPixels(pixelsToPoints(1234, 2.0), 2.0), 1234);
}

TEST(WindowGeometry, UnlimitedMaxSizeStaysUnlimited) {
    // regression: -1 / dpi used to reach NSWindow as a negative size, clamping the window to 0x0
    EXPECT_EQ(maxPixelsToPoints(-1, 2.0), UNLIMITED_POINTS);
    EXPECT_EQ(maxPixelsToPoints(-1, 1.0), UNLIMITED_POINTS);
}

TEST(WindowGeometry, LimitedMaxSizeIsConverted) {
    EXPECT_DOUBLE_EQ(maxPixelsToPoints(1000, 2.0), 500.0);
    EXPECT_DOUBLE_EQ(maxPixelsToPoints(0, 2.0), 0.0);
}
