// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
