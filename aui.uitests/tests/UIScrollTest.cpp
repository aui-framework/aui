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

#include "AUI/ASS/Property/MinSize.h"
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AScrollArea.h>
#include <gmock/gmock.h>

class MockedViewContainer: public AViewContainer {
public:
    MockedViewContainer() {
        ON_CALL(*this, setSize).WillByDefault([this](glm::ivec2 size) {
            AViewContainer::setSize(size);
        });

        setContents(
            Vertical {
                mBigContent = _new<ALabel>("Content") AUI_WITH_STYLE { ass::MinSize{ {}, 300_dp } },
                mBottomLabel = _new<ALabel>("Bottom"),
            }
        );
        *this << ".container";
        setCustomStyle({ ass::BackgroundSolid{ AColor::BLUE } });
    
        EXPECT_CALL(*this, setSize(testing::_)).Times(testing::AtLeast(1));
    }

    MOCK_METHOD(void, setSize, (glm::ivec2), (override));
    _<ALabel> mBigContent;
    _<ALabel> mBottomLabel;
};

class UIScrollTest: public testing::UITest {
public:
protected:

    class TestWindow: public AWindow {
    public:
        TestWindow(): AWindow("Test window", 200_dp, 100_dp) {
            setContents(Vertical {
                    mScrollArea = AScrollArea::Builder().withContents(mMockedContainer = _new<MockedViewContainer>()).withExpanding().build() AUI_WITH_STYLE {
                        ass::MinSize(100_dp),
                    }
            });
        }
        _<AScrollArea> mScrollArea;
        _<MockedViewContainer> mMockedContainer;
    };
    _<TestWindow> mTestWindow;


    void SetUp() override {
        UITest::SetUp();

        // prepare the window
        mTestWindow = _new<TestWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        // delete mocked objects
        mTestWindow->removeAllViews();
        AThread::processMessages();
        UITest::TearDown();
    }
};

TEST_F(UIScrollTest, ContainedViewExpanded) {
    By::type<AScrollArea>().perform(scroll({0, 500}));
    (By::name(".container") | By::type<AScrollArea>()).check(areBottomAligned());
}

TEST_F(UIScrollTest, ScrollTo1) {
    mTestWindow->applyGeometryToChildrenIfNecessary();
    By::text("Bottom").check(uitest::impl::not$(isBottomAboveBottomOf(By::type<AScrollArea>())));
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBottomLabel);
    By::text("Bottom").check(isBottomAboveBottomOf(By::type<AScrollArea>()));
}

TEST_F(UIScrollTest, ScrollTo2) {
    mTestWindow->applyGeometryToChildrenIfNecessary();
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBottomLabel);
    By::text("Content").check(isTopAboveTopOf(By::type<AScrollArea>()), "first check");
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBigContent);
    By::text("Content").check(uitest::impl::not$(isTopAboveTopOf(By::type<AScrollArea>())), "second check");
}
