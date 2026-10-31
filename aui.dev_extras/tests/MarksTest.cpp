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

#include "AUI/Remote/Agent/Marks.h"

using namespace aui::remote;

namespace {
const AColor BACKGROUND = 0x808080_rgb;

AImage blank(glm::uvec2 size) {
    AImage image(size, APixelFormat::RGBA_BYTE);
    image.fill(BACKGROUND);
    return image;
}

ARect<int> rect(int x, int y, int w, int h) { return ARect<int>::fromTopLeftPositionAndSize({ x, y }, { w, h }); }
}   // namespace

TEST(MarksTest, BoxIsOutlinedAndItsInsideStaysVisible) {
    auto image = blank({ 300, 200 });
    marks::draw(image, { { 1, rect(100, 80, 120, 80) } }, 1.f);
    EXPECT_NE(image.get({ 100, 120 }), BACKGROUND);   // left edge of the box
    EXPECT_EQ(image.get({ 160, 140 }), BACKGROUND);   // the middle of the box is not covered
    EXPECT_EQ(image.get({ 10, 190 }), BACKGROUND);    // far from the box
}

TEST(MarksTest, MarksAtTheEdgesAndLongNumbersStayInside) {
    auto image = blank({ 120, 80 });
    marks::draw(image,
                {
                  { 1, rect(0, 0, 30, 20) },
                  { 123, rect(90, 60, 30, 20) },
                },
                2.f);
    EXPECT_NE(image.get({ 0, 0 }), BACKGROUND);
    EXPECT_NE(image.get({ 119, 79 }), BACKGROUND);
}

TEST(MarksTest, PlatesOfNeighboursDontCoverEachOther) {
    // two boxes side by side: if both numbers were drawn at the same spot, the first one would be overwritten
    auto image = blank({ 200, 100 });
    marks::draw(image,
                {
                  { 1, rect(50, 40, 10, 20) },
                  { 2, rect(52, 40, 10, 20) },
                },
                1.f);
    auto alone = blank({ 200, 100 });
    marks::draw(alone, { { 1, rect(50, 40, 10, 20) } }, 1.f);
    int differences = 0;
    for (unsigned y = 0; y < 40; ++y) {   // the plates go above the boxes
        for (unsigned x = 0; x < 200; ++x) {
            differences += alone.get({ x, y }) != image.get({ x, y }) && alone.get({ x, y }) != BACKGROUND;
        }
    }
    EXPECT_EQ(differences, 0) << "the plate of mark 1 is covered";
}
