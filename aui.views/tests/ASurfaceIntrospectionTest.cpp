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
#include "AUI/View/ATextField.h"
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

TEST_F(ASurfaceIntrospectionTest, WindowWithOverlappingSurfaceIsDestroyed) {
    auto window = _new<AWindow>();
    window->setContents(Centered { Label { "text" } });
    window->show();
    uitest::frame();
    auto popup = window->createOverlappingSurface({ 10, 10 }, { 50, 50 });
    uitest::frame();

    // used to call the pure virtual closeOverlappingSurfaceImpl from ~ASurface
    window->close();
    auto weak = window.weak();
    window = nullptr;
    uitest::frame();
    EXPECT_TRUE(weak.expired());
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
    window->injectEvent(APointerPressedEvent { center, APointerIndex::button(AInput::LBUTTON) });
    window->injectEvent(APointerReleasedEvent { center, APointerIndex::button(AInput::LBUTTON) });
    uitest::frame();
    EXPECT_EQ(clicked, 1);
}

TEST_F(ASurfaceIntrospectionTest, InjectedKeysAndCharsReachFocusedView) {
    auto window = _new<AWindow>();
    auto field = _new<ATextField>();
    window->setContents(Centered { field });
    window->show();
    uitest::frame();

    const auto center = glm::vec2(field->getCenterPointInWindow());
    window->injectEvent(APointerPressedEvent { center, APointerIndex::button(AInput::LBUTTON) });
    window->injectEvent(APointerReleasedEvent { center, APointerIndex::button(AInput::LBUTTON) });
    uitest::frame();

    window->injectEvent(AInjectedChar { 'h' });
    window->injectEvent(AInjectedChar { 'i' });
    uitest::frame();
    EXPECT_EQ(field->getText(), "hi");

    window->injectEvent(AInjectedKey { AInjectedKey::Action::DOWN, AInput::BACKSPACE });
    window->injectEvent(AInjectedKey { AInjectedKey::Action::UP, AInput::BACKSPACE });
    uitest::frame();
    EXPECT_EQ(field->getText(), "h");
}

TEST_F(ASurfaceIntrospectionTest, InjectedMoveHoversViews) {
    auto window = _new<AWindow>();
    auto button = _new<AButton>("Hover me");
    window->setContents(Centered { button });
    window->show();
    uitest::frame();
    EXPECT_FALSE(button->isMouseEntered());

    window->injectEvent(AInjectedPointerMove { glm::vec2(button->getCenterPointInWindow()) });
    uitest::frame();
    EXPECT_TRUE(button->isMouseEntered());

    window->injectEvent(AInjectedPointerMove { glm::vec2(1, 1) });
    uitest::frame();
    EXPECT_FALSE(button->isMouseEntered());
}
