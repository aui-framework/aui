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

class ViewMock: public AView {
public:
    ViewMock() {
        setCustomStyle({
            ass::MinSize{ 16_dp },
            ass::BackgroundSolid{ AColor::RED },
        });

        ON_CALL(*this, onPointerPressed).WillByDefault([this](const auto& a) {
            AView::onPointerPressed(a);
        });
        ON_CALL(*this, onPointerReleased).WillByDefault([this](const auto& a) {
            AView::onPointerReleased(a);
        });

        connect(clicked, me::onClicked);
    }

    MOCK_METHOD(void, onPointerPressed, (const APointerPressedEvent& event), (override));
    MOCK_METHOD(void, onPointerReleased, (const APointerReleasedEvent& event), (override));
    MOCK_METHOD(void, onClicked, (), ());


};


class UIPointerBehaviour: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        ALayoutInflater::inflate(mWindow, mView = _new<ViewMock>());
        mWindow->show();
    }

    void TearDown() override {
        mWindow = nullptr;
        mView = nullptr;
        UITest::TearDown();
    }

    _<AWindow> mWindow;
    _<ViewMock> mView;
};


/**
 * Checks basic click logic.
 */
TEST_F(UIPointerBehaviour, ClickTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_));
    EXPECT_CALL(*mView, onPointerReleased(testing::_));
    EXPECT_CALL(*mView, onClicked());
    By::type<ViewMock>().perform(click());
}


/**
 * Checks click behaviour when the click is appeared outside the view.
 */
TEST_F(UIPointerBehaviour, ClickOutsideTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_));
    EXPECT_CALL(*mView, onPointerReleased(testing::_));
    EXPECT_CALL(*mView, onClicked()).Times(0);
    mWindow->onPointerPressed({
        .position = { 10, 10 }, // somewhere over the mView
        .button = AInput::LBUTTON,
    });

    mWindow->onPointerMove(
        { 100, 100 } // somewhere outside the mView
    );
    mWindow->onPointerReleased({
        .position = { 100, 100 }, // somewhere outside the mView
        .button = AInput::LBUTTON,
    });
}
