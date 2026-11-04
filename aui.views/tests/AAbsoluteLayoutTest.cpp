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
#include <AUI/Layout/AAbsoluteLayout.h>
#include <AUI/View/AViewContainer.h>
#include <AUI/View/ALabel.h>

namespace {
_<AViewContainer> absoluteContainer() {
    auto container = _new<AViewContainer>();
    container->setLayout(std::make_unique<AAbsoluteLayout>());
    return container;
}
}

// a view added without a pivot is positioned manually and must not stretch its container (i.e., to the width of its
// text).
TEST(AAbsoluteLayoutTest, ManuallyPositionedViewDoesNotContributeToSize) {
    auto container = absoluteContainer();
    auto label = _new<ALabel>("a pretty long text which is wider than anything");
    container->addView(label);
    label->layout({ 0, 500 }, { 10, 10 });

    EXPECT_EQ(container->computeMinMaxAxis().min, 0);
    EXPECT_EQ(container->computeMinMaxAxis().max, 0);
    EXPECT_EQ(container->measure({}), glm::ivec2(0));

    container->layout({ 0, 0 }, { 100, 100 });
    EXPECT_EQ(label->getPosition(), glm::ivec2(0, 500));
    EXPECT_EQ(label->getSize(), glm::ivec2(10, 10));
}

TEST(AAbsoluteLayoutTest, PivotedViewContributesToSize) {
    auto container = absoluteContainer();
    auto label = _new<ALabel>("text");
    dynamic_cast<AAbsoluteLayout&>(*container->getLayout())
        .add({ .view = label, .pivotX = 10_px, .pivotY = 20_px, .sizeX = 30_px, .sizeY = 40_px });

    EXPECT_EQ(container->computeMinMaxAxis().min, 40);
    EXPECT_EQ(container->measure({}), glm::ivec2(40, 60));
}
