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


class UIStyleTest: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        using namespace declarative;
        ALayoutInflater::inflate(mWindow,
                                 Vertical {
                                         mView = _new<AView>() with_style {
                                             BackgroundSolid { AColor::BLACK },
                                             on_state::Hovered {
                                                 BackgroundSolid { AColor::RED },
                                             },
                                         },
                                         Label { "Some bullshit to complicate layout" },
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

/**
 * Checks mouse move events.
 */
TEST_F(UIStyleTest, MouseMoveNoClick) {
    testing::InSequence s;


    mWindow->onPointerMove({ 10, 10 }); // somewhere over the mView
    mWindow->onPointerMove({ 100, 100 }); // somewhere outside the mView
}