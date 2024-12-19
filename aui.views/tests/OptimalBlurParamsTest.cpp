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
#include "AUI/ASS/Property/Backdrop.h"

using GaussianBlurCustom = ass::Backdrop::GaussianBlurCustom;
using GaussianBlur = ass::Backdrop::GaussianBlur;

namespace ass {
static std::ostream& operator<<(std::ostream& os, const GaussianBlurCustom& v) {
    return os << "GaussianBlurCustom { .radius = " << v.radius << ", .downscale = " << v.downscale << " }";
}
}

TEST(OptimalBlurParamsTest, Radius_1) {
    auto actual = GaussianBlur{ .radius = 1_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 1_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_2) {
    auto actual = GaussianBlur{ .radius = 2_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 2_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_3) {
    auto actual = GaussianBlur{ .radius = 3_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_4) {
    auto actual = GaussianBlur{ .radius = 4_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 4_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_5) {
    auto actual = GaussianBlur{ .radius = 5_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 5_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_6) {
    auto actual = GaussianBlur{ .radius = 6_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 2 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_7) {
    auto actual = GaussianBlur{ .radius = 7_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 7_px, .downscale = 1 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_8) {
    auto actual = GaussianBlur{ .radius = 8_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 4_px, .downscale = 2 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_9) {
    auto actual = GaussianBlur{ .radius = 9_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 3 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_10) {
    auto actual = GaussianBlur{ .radius = 10_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 5_px, .downscale = 2 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_11) {
    auto actual = GaussianBlur{ .radius = 11_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 5_px, .downscale = 2 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_12) {
    auto actual = GaussianBlur{ .radius = 12_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 4 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_13) {
    auto actual = GaussianBlur{ .radius = 13_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 4 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_14) {
    auto actual = GaussianBlur{ .radius = 14_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 7_px, .downscale = 2 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_15) {
    auto actual = GaussianBlur{ .radius = 15_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 5 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_16) {
    auto actual = GaussianBlur{ .radius = 16_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 4_px, .downscale = 4 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_17) {
    auto actual = GaussianBlur{ .radius = 17_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 4_px, .downscale = 4 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_18) {
    auto actual = GaussianBlur{ .radius = 18_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 6 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_19) {
    auto actual = GaussianBlur{ .radius = 19_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 3_px, .downscale = 6 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_20) {
    auto actual = GaussianBlur{ .radius = 20_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 4_px, .downscale = 5 };
    EXPECT_EQ(actual, expected);
}

TEST(OptimalBlurParamsTest, Radius_50) {
    auto actual = GaussianBlur{ .radius = 50_px }.findOptimalParams();
    auto expected = GaussianBlurCustom { .radius = 5_px, .downscale = 10 };
    EXPECT_EQ(actual, expected);
}
