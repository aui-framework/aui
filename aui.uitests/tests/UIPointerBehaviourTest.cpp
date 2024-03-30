// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
class ViewMock: public AView {
public:
    ViewMock() {
        setCustomStyle({
            ass::MinSize{ 32_dp },
            ass::BackgroundSolid{ AColor::RED },
        });

        ON_CALL(*this, onPointerPressed).WillByDefault([this](const auto& a) {
            AView::onPointerPressed(a);
        });
        ON_CALL(*this, onPointerReleased).WillByDefault([this](const auto& a) {
            AView::onPointerReleased(a);
        });

        ON_CALL(*this, onMouseEnter).WillByDefault([this]() {
            AView::onMouseEnter();
        });
        ON_CALL(*this, onPointerMove).WillByDefault([this](const auto&... a) {
            AView::onPointerMove(a...);
        });
        ON_CALL(*this, onMouseLeave).WillByDefault([this]() {
            AView::onMouseLeave();
        });

        connect(clicked, me::onClicked);
    }

    MOCK_METHOD(void, onPointerPressed, (const APointerPressedEvent& event), (override));
    MOCK_METHOD(void, onPointerReleased, (const APointerReleasedEvent& event), (override));
    MOCK_METHOD(void, onClicked, (), ());
    MOCK_METHOD(void, onDummyTest, (), ());

    MOCK_METHOD(void, onMouseEnter, (), (override));
    MOCK_METHOD(void, onPointerMove, (glm::vec2 pos, const APointerMoveEvent& e), (override));
    MOCK_METHOD(void, onMouseLeave, (), (override));
};
}


class UIPointerBehaviour: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        using namespace declarative;
        ALayoutInflater::inflate(mWindow,
            Vertical {
              Centered {
                mContainer = Vertical {
                  mView = _new<ViewMock>(),
                  Label { "Some bullshit to complicate layout" },
                },
                mOverlay = Vertical::Expanding {
                  SpacerExpanding{},
                  Button { "Another bullshit" }
                },
              }
            });
        mWindow->show();
        uitest::frame();
    }

    void TearDown() override {
        mWindow = nullptr;
        mView = nullptr;
        UITest::TearDown();
    }

    _<AWindow> mWindow;
    _<AViewContainer> mContainer;
    _<AViewContainer> mOverlay;
    _<ViewMock> mView;
};


/**
 * Checks basic click logic.
 */
TEST_F(UIPointerBehaviour, ClickTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_));
    EXPECT_CALL(*mView, onDummyTest());
    EXPECT_CALL(*mView, onPointerReleased(testing::_));
    EXPECT_CALL(*mView, onClicked());

    AObject::connect(mContainer->clicked, slot(mView)::onDummyTest);

    By::type<ViewMock>().perform(click());
}


/**
 * Checks click behaviour when the click is appeared outside the view.
 */
TEST_F(UIPointerBehaviour, ClickOutsideTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_));
    EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_));
    EXPECT_CALL(*mView, onPointerReleased(testing::_));
    EXPECT_CALL(*mView, onClicked()).Times(0);

    mWindow->onPointerPressed({
        .position = mView->getCenterPointInWindow(), // somewhere over the mView
    });

    mWindow->onPointerMove(
        { 100, 100 } // somewhere outside the mView
    , {});
    mWindow->onPointerReleased({
        .position = { 100, 100 }, // somewhere outside the mView
    });
}

/**
 * Checks mouse move events.
 */
TEST_F(UIPointerBehaviour, MouseMoveNoClick) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_)).Times(0);
    EXPECT_CALL(*mView, onPointerReleased(testing::_)).Times(0);
    EXPECT_CALL(*mView, onClicked()).Times(0);

    EXPECT_CALL(*mView, onMouseEnter);
    EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mView, onMouseLeave);

    mWindow->onPointerMove(mView->getCenterPointInWindow(), {}); // somewhere over the mView
    mWindow->onPointerMove({ 100, 100 }, {}); // somewhere outside the mView
}


/**
 * Checks multiple button pointerPressed/Released behaviour.
 */
TEST_F(UIPointerBehaviour, MultiplePointerPressedReleased) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_)).Times(2);
    EXPECT_CALL(*mView, onPointerReleased(testing::_)).Times(2);

    mWindow->onPointerPressed({
      .position = mView->getCenterPointInWindow(), // somewhere over the mView
      .pointerIndex = APointerIndex::button(AInput::LBUTTON),
    });
    mWindow->onPointerPressed({
      .position = mView->getCenterPointInWindow(), // somewhere over the mView
      .pointerIndex = APointerIndex::button(AInput::RBUTTON),
    });
    mWindow->onPointerReleased({
      .position = mView->getCenterPointInWindow(), // somewhere over the mView
      .pointerIndex = APointerIndex::button(AInput::RBUTTON),
    });
    mWindow->onPointerReleased({
      .position = mView->getCenterPointInWindow(), // somewhere over the mView
      .pointerIndex = APointerIndex::button(AInput::LBUTTON),
    });
}


/**
 * Checks that "Another bullshit" button is pressed.
 */
TEST_F(UIPointerBehaviour, FrontLayerClickTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_)).Times(0);
    EXPECT_CALL(*mView, onPointerReleased(testing::_)).Times(0);
    EXPECT_CALL(*mView, onDummyTest()).Times(1);

    auto anotherBullshit = By::text("Another bullshit");
    AObject::connect(anotherBullshit.one()->clicked, slot(mView)::onDummyTest);

    anotherBullshit.perform(click());
}


/**
 * Checks that ViewMock is not reachable as there's overlay in front of it.
 */
TEST_F(UIPointerBehaviour, ClickOverlayTest) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onPointerPressed(testing::_)).Times(0);
    EXPECT_CALL(*mView, onPointerReleased(testing::_)).Times(0);
    EXPECT_CALL(*mView, onClicked()).Times(0);

    mOverlay->setCustomStyle({
        ass::BackgroundSolid(AColor::WHITE)
    });

    By::type<ViewMock>().perform(click());
}
