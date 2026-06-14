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
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/AScrollArea.h"

namespace {
class ViewMock: public AView {
public:
    ViewMock() {
        ON_CALL(*this, onMouseEnter).WillByDefault([this]() {
            AView::onMouseEnter();
        });
        ON_CALL(*this, onPointerMove).WillByDefault([this](const auto&... a) {
            AView::onPointerMove(a...);
        });
        ON_CALL(*this, onMouseLeave).WillByDefault([this]() {
            AView::onMouseLeave();
        });
    }

    MOCK_METHOD(void, onMouseEnter, (), (override));
    MOCK_METHOD(void, onPointerMove, (glm::vec2 pos, const APointerMoveEvent& e), (override));
    MOCK_METHOD(void, onMouseLeave, (), (override));
};
}

class UIScrollPointerMove: public testing::UITest {
public:
protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>();
        using namespace declarative;
        ALayoutInflater::inflate(mWindow,
                                 Vertical {
                                   Centered {
                                     AScrollArea::Builder().withContents(Vertical {
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       mView = _new<ViewMock>() AUI_OVERRIDE_STYLE {
                                         MinSize { 16_dp },
                                         BackgroundSolid { AColor::BLACK },
                                         on_state::Hovered {
                                             BackgroundSolid { AColor::RED },
                                         },
                                       },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                       Label { "Some bullshit" },
                                     }).build() AUI_OVERRIDE_STYLE {
                                       MinSize { 100_dp },
                                     }
                                   }
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
    _<ViewMock> mView;
};


/**
 * Checks mouse move events along with scroll.
 */
TEST_F(UIScrollPointerMove, MouseMoveScroll) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onMouseEnter);
    EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mView, onMouseLeave);

    By::type<ViewMock>().check(averageColor(AColor::BLACK));
    mWindow->onPointerMove(mView->getCenterPointInWindow(), {}); // somewhere over the mView
    By::type<ViewMock>().check(averageColor(AColor::RED));
    mWindow->onScroll(AScrollEvent {
        .origin  = mView->getCenterPointInWindow(),
        .delta   = { 0, 30 },
        .kinetic = false,
    });
    By::type<ViewMock>().check(averageColor(AColor::BLACK), "view didn't become black");
}


/**
 * Same as previous, but scrolls back again to check view's style still changing on hover.
 */
TEST_F(UIScrollPointerMove, MouseMoveScrollAndBack) {
    testing::InSequence s;

    EXPECT_CALL(*mView, onMouseEnter);
    EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mView, onMouseLeave);

    EXPECT_CALL(*mView, onMouseEnter);
    EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mView, onMouseLeave);

    By::type<ViewMock>().check(averageColor(AColor::BLACK));
    mWindow->onPointerMove(mView->getCenterPointInWindow(), {}); // somewhere over the mView
    By::type<ViewMock>().check(averageColor(AColor::RED));
    mWindow->onScroll(AScrollEvent {
        .origin  = mView->getCenterPointInWindow(),
        .delta   = { 0, 30 },
        .kinetic = false,
    });
    By::type<ViewMock>().check(averageColor(AColor::BLACK), "view didn't become black");

    mWindow->onScroll(AScrollEvent {
        .origin  = mView->getCenterPointInWindow(),
        .delta   = { 0, -30 },
        .kinetic = false,
    });

    mWindow->onPointerMove(mView->getCenterPointInWindow(), {}); // somewhere over the mView
    By::type<ViewMock>().check(averageColor(AColor::RED), "view didn't become red after returning back");


    mWindow->onPointerMove({100, 100}, {}); // somewhere outside the mView
    By::type<ViewMock>().check(averageColor(AColor::BLACK), "view didn't become black");
}


/**
 * Checking onMouseEnter and onMouseLeave events when left mouse button is pressed
 */
TEST_F(UIScrollPointerMove, MouseDownMoveAndBack) {
    constexpr glm::ivec2 EPS(1, 0);
    constexpr glm::ivec2 posOutOfView = {100, 100};

    //moving to view
    {
        testing::InSequence s;
        EXPECT_CALL(*mView, onMouseEnter);
        EXPECT_CALL(*mView, onPointerMove(testing::_, testing::_)).Times(testing::AtLeast(1));
        By::type<ViewMock>().check(averageColor(AColor::BLACK));
        mWindow->onPointerMove(mView->getCenterPointInWindow(), {}); // somewhere over the mView
        By::type<ViewMock>().check(averageColor(AColor::RED));
    }

    //pressing LMB and moving out of view
    {
        testing::InSequence s;

        EXPECT_CALL(*mView, onMouseLeave);

        APointerPressedEvent event;        
        event.position = mView->getCenterPointInWindow();
        event.asButton = AInput::LBUTTON;
        mWindow->onPointerPressed(event);
        mWindow->onPointerMove(posOutOfView, {});
    }

    //just some movements out of view
    {
        testing::InSequence s;
        EXPECT_CALL(*mView, onMouseEnter).Times(0);
        mWindow->onPointerMove(posOutOfView - EPS, {});
        mWindow->onPointerMove(posOutOfView + EPS, {});
    }

    //moving back to view, some movements inside the view and releasing LMB
    {
        testing::InSequence s;
        EXPECT_CALL(*mView, onMouseEnter).Times(1);
        mWindow->onPointerMove(mView->getCenterPointInWindow() - EPS, {});
        mWindow->onPointerMove(mView->getCenterPointInWindow() + EPS, {});
        mWindow->onPointerMove(mView->getCenterPointInWindow(), {});

        APointerReleasedEvent event;
        event.position = mView->getCenterPointInWindow();
        event.asButton = AInput::LBUTTON;
        mWindow->onPointerReleased(event);
    }

    //checking everything is fine after releasing LMB
    {
        testing::InSequence s;
        EXPECT_CALL(*mView, onMouseLeave);
        EXPECT_CALL(*mView, onMouseEnter);
        mWindow->onPointerMove(posOutOfView, {});
        mWindow->onPointerMove(mView->getCenterPointInWindow(), {});
    }

}
