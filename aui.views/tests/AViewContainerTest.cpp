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
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AViewContainer.h>

using namespace declarative;

// A view moved into another container, the way a declarative container takes an existing view: the old container must
// let it go, and its death must not reset the parent of the view (otherwise the redraw() of the view goes nowhere and
// it gets a frame only when something else redraws the window).

TEST(AViewContainer, DeclarativeContainerTakesTheViewFromTheOldOne) {
    auto view = _new<AView>();
    _<AViewContainer> old = Stacked { view };
    _<AViewContainer> moved = Stacked { view };
    EXPECT_TRUE(old->getViews().empty());
    EXPECT_EQ(view->getParent(), moved.get());
    old = nullptr;
    EXPECT_EQ(view->getParent(), moved.get());
}

TEST(AViewContainer, AddViewsTakesTheViewFromTheOldOne) {
    auto view = _new<AView>();
    auto old = _new<AViewContainer>();
    old->addView(view);
    auto moved = _new<AViewContainer>();
    moved->addViews({ view });
    EXPECT_TRUE(old->getViews().empty());
    old = nullptr;
    EXPECT_EQ(view->getParent(), moved.get());
}

TEST(AViewContainer, AddViewAtIndexTakesTheViewFromTheOldOne) {
    auto view = _new<AView>();
    auto old = _new<AViewContainer>();
    old->addView(view);
    auto moved = _new<AViewContainer>();
    moved->addView(0, view);
    EXPECT_TRUE(old->getViews().empty());
    old = nullptr;
    EXPECT_EQ(view->getParent(), moved.get());
}

TEST(AViewContainer, SetViewsKeepsTheViewsOfItsOwn) {
    auto a = _new<AView>();
    auto b = _new<AView>();
    _<AViewContainer> container = Stacked { a, b };
    container->setViews({ b, a });
    ASSERT_EQ(container->getViews().size(), 2);
    EXPECT_EQ(container->getViews()[0], b);
    EXPECT_EQ(container->getViews()[1], a);
    EXPECT_EQ(a->getParent(), container.get());
    EXPECT_EQ(b->getParent(), container.get());
}

TEST(AViewContainer, ClearingTheOldOneDoesNotResetTheNewParent) {
    auto view = _new<AView>();
    _<AViewContainer> old = Stacked { view };
    auto moved = _new<AViewContainer>();
    moved->addView(view);
    old->removeAllViews();
    EXPECT_EQ(view->getParent(), moved.get());
}
