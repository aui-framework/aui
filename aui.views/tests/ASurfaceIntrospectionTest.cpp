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

#include <gmock/gmock.h>
#include "AUI/UITest.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AButton.h"
#include "AUI/Platform/AOverlappingSurface.h"

using namespace declarative;

class ASurfaceIntrospectionTest : public testing::UITest {};

TEST_F(ASurfaceIntrospectionTest, PendingLayoutIsReported) {
    auto window = _new<AWindow>();
    window->setContents(Centered { Label { "text" } });
    window->show();
    uitest::frame();
    EXPECT_FALSE(window->hasPendingLayout());

    window->requestLayout();
    EXPECT_TRUE(window->hasPendingLayout());

    uitest::frame();
    EXPECT_FALSE(window->hasPendingLayout());
}

TEST_F(ASurfaceIntrospectionTest, OverlappingSurfacesAreListed) {
    auto window = _new<AWindow>();
    window->setContents(Centered { Label { "text" } });
    window->show();
    uitest::frame();
    EXPECT_TRUE(window->overlappingSurfaces().empty());

    auto popup = window->createOverlappingSurface({ 10, 10 }, { 50, 50 });
    ASSERT_EQ(window->overlappingSurfaces().size(), 1u);
    EXPECT_TRUE(window->overlappingSurfaces().contains(popup));
    uitest::frame();   // showing the surface is deferred

    window->closeOverlappingSurface(popup);
    EXPECT_TRUE(window->overlappingSurfaces().empty());
}

TEST_F(ASurfaceIntrospectionTest, InjectedPressReachesViews) {
    auto window = _new<AWindow>();
    auto clicked = 0;
    auto button = _new<AButton>("Click me");
    AObject::connect(button->clicked, window, [&] { ++clicked; });
    window->setContents(Centered { button });
    window->show();
    uitest::frame();

    const auto center = glm::vec2(button->getCenterPointInWindow());
    window->injectPointerPressed({ center, APointerIndex::button(AInput::LBUTTON) });
    window->onPointerReleased({ center, APointerIndex::button(AInput::LBUTTON) });
    uitest::frame();
    EXPECT_EQ(clicked, 1);
}
