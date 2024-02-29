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

#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/Event/AScrollEvent.h"
#include "AUI/UITest.h"
#include "AUI/Util/ClipOptimizationContext.h"
#include "AUI/View/AScrollAreaInner.h"
#include "glm/fwd.hpp"
#include <gmock/gmock-cardinalities.h>
#include <gmock/gmock.h>
#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <AUI/View/AScrollArea.h>
#include <gtest/gtest.h>

/**
 * This test suite checks that views that 100% should not be rendered (i.e. outside of scroll area bounding box) are
 * not actually rendered.
 */

namespace {
    class ViewMock: public AView {
    public:
        MOCK_METHOD(void, render, (ClipOptimizationContext), ());
    };
}


class UIRenderOptimizationTest: public testing::UITest {
    public:
    protected:

        class TestWindow: public AWindow {
        public:
            _<ViewMock> mTop;
            _<ViewMock> mCenter;
            _<ViewMock> mBottom;
            _<AScrollArea> mScroll;
            TestWindow() {
                setContents(Centered {
                    mScroll = AScrollArea::Builder().withContents(Vertical {
                        mTop = _new<ViewMock>() with_style { ass::FixedSize { 10_px } },
                        // big enough to push away bottom view
                        mCenter = _new<ViewMock>() with_style { ass::FixedSize { 1000_px } }, 
                        mBottom = _new<ViewMock>() with_style { ass::FixedSize { 10_px } },
                    }).build() with_style { ass::FixedSize(200_px) },
                });

                pack();
            }
        };

    void SetUp() override {
        UITest::SetUp();

        mTestWindow = _new<TestWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        mTestWindow = nullptr;
        UITest::TearDown();
    }

    _<TestWindow> mTestWindow;
};

/**
 * Checks that in beginning of the scrollarea, bottom view is not rendered.
 */
TEST_F(UIRenderOptimizationTest, BeginningOfScrollArea) {
    EXPECT_CALL(*mTestWindow->mTop, render(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mTestWindow->mCenter, render(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mTestWindow->mBottom, render(testing::_)).Times(testing::Exactly(0));
    ON_CALL(*mTestWindow->mBottom, render(testing::_)).WillByDefault([] {
        // place for a breakpoint.
        GTEST_FAIL();
    });

    uitest::frame();
}

/**
 * Checks that in centered of the scrollarea, both top and bottom views are not rendered.
 */
TEST_F(UIRenderOptimizationTest, CenterOfScrollArea) {
    mTestWindow->mScroll->scroll({0, 100});

    EXPECT_CALL(*mTestWindow->mTop, render(testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*mTestWindow->mCenter, render(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mTestWindow->mBottom, render(testing::_)).Times(testing::Exactly(0));
    ON_CALL(*mTestWindow->mTop, render(testing::_)).WillByDefault([] {
        // place for a breakpoint.
        GTEST_FAIL();
    });
    ON_CALL(*mTestWindow->mBottom, render(testing::_)).WillByDefault([] {
        // place for a breakpoint.
        GTEST_FAIL();
    });

    uitest::frame();
}

/**
 * Checks that in the end of the scrollarea, top view is not rendered.
 */
TEST_F(UIRenderOptimizationTest, EndOfScrollArea) {
    mTestWindow->mScroll->scroll({0, 10000});

    EXPECT_CALL(*mTestWindow->mTop, render(testing::_)).Times(testing::Exactly(0));
    EXPECT_CALL(*mTestWindow->mCenter, render(testing::_)).Times(testing::AtLeast(1));
    EXPECT_CALL(*mTestWindow->mBottom, render(testing::_)).Times(testing::AtLeast(0));
    ON_CALL(*mTestWindow->mTop, render(testing::_)).WillByDefault([] {
        // place for a breakpoint.
        GTEST_FAIL();
    });

    uitest::frame();
}