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
                mBigContent = _new<ALabel>("Content") with_style { ass::MinSize{ {}, 300_dp } },
                mBottomLabel = _new<ALabel>("Bottom"),
            }
        );
        *this << ".container";
        setCustomStyle({ ass::BackgroundSolid{ 0xff0000_rgb, } });
    
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
                    mScrollArea = AScrollArea::Builder().withContents(mMockedContainer = _new<MockedViewContainer>()).withExpanding().build() with_style {
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
    // scroll a little to see whether container view expanded properly
    mTestWindow->updateLayout();
    By::type<AScrollArea>().perform(scroll({0, 500}));
    By::name(".container").check(isBottomAboveBottomOf(By::type<AScrollArea>()));
}

TEST_F(UIScrollTest, ScrollTo) {
    mTestWindow->updateLayout();
    By::text("Bottom").check(uitest::impl::not$(isBottomAboveBottomOf(By::type<AScrollArea>())));
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBottomLabel);
    By::text("Bottom").check(isBottomAboveBottomOf(By::type<AScrollArea>()));
}

TEST_F(UIScrollTest, ScrollTo2) {
    mTestWindow->updateLayout();
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBottomLabel);
    By::text("Content").check(isTopAboveTopOf(By::type<AScrollArea>()), "first check");
    mTestWindow->mScrollArea->scrollTo(mTestWindow->mMockedContainer->mBigContent);
    By::text("Content").check(uitest::impl::not$(isTopAboveTopOf(By::type<AScrollArea>())), "second check");
}
