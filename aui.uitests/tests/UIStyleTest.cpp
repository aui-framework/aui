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

#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "AUI/Util/ALayoutInflater.h"


namespace {

    class View : public AView {
    };

    class UIStyleTest : public testing::UITest {
    public:
    protected:
        void SetUp() override {
            UITest::SetUp();

            mWindow = _new<AWindow>();
            using namespace declarative;
            ALayoutInflater::inflate(mWindow,
                                     Vertical{
                                             mView = _new<View>() with_style {
                                                     MinSize { 10_dp },
                                                     BackgroundSolid{AColor::BLACK},

                                                     on_state::Hovered {
                                                         BackgroundSolid{AColor::RED},
                                                     },

                                                     on_state::Activated {
                                                         BackgroundSolid{AColor::GREEN},
                                                     },
                                             },
                                             Label{"Some bullshit to complicate layout"},
                                     }
            );
            mWindow->show();
        }

        void TearDown() override {
            mWindow = nullptr;
            mView = nullptr;
            UITest::TearDown();
        }

        _<AWindow> mWindow;
        _<AView> mView;
    };
}

/**
 * Checks mouse move events.
 */
TEST_F(UIStyleTest, MouseMoveNoClick) {
    testing::InSequence s;

    By::type<View>().check(averageColor(AColor::BLACK));

    mWindow->onPointerMove({ 10, 10 }, {}); // somewhere over the mView
    By::type<View>().check(averageColor(AColor::RED));

    mWindow->onPointerMove({ 100, 100 }, {}); // somewhere outside the mView
    By::type<View>().check(averageColor(AColor::BLACK));
}


/**
 * Checks click with release outside the view.
 */
TEST_F(UIStyleTest, MouseMoveWithClick) {
    testing::InSequence s;

    By::type<View>().check(averageColor(AColor::BLACK));

    mWindow->onPointerMove({ 10, 10 }, {}); // somewhere over the mView
    By::type<View>().check(averageColor(AColor::RED));

    mWindow->onPointerPressed({.position = { 10, 10 } }); // somewhere over the mView
    By::type<View>().check(averageColor(AColor::GREEN));

    mWindow->onPointerMove({ 100, 100 }, {}); // somewhere outside the mView
    By::type<View>().check(averageColor(AColor::GREEN));

    mWindow->onPointerReleased({.position = { 100, 100 }, .triggerClick = false }); // somewhere outside the view
    By::type<View>().check(averageColor(AColor::BLACK));
}

/**
 * Checks click with release outside the view.
 */
TEST_F(UIStyleTest, Opacity) {
    testing::InSequence s;
    using namespace ass;

    mView with_style {
        FixedSize(50_dp),
        BackgroundSolid(AColor::RED),
        Opacity(0),
    };
    By::type<View>().check(averageColor(AColor::WHITE), "view should disappear");
    mView with_style {
        FixedSize(50_dp),
        BackgroundSolid(AColor::RED),
        Opacity(1),
    };
    By::type<View>().check(averageColor(AColor::RED), "view should appear");
}
