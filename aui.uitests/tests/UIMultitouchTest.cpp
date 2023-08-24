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
#include "AUI/View/ASlider.h"

/**
 * This test suite checks a simple program.
 * There's a button "Say hello":
 * ___________-[]X_
 * |              |
 * |  [Say hello] |
 * |              |
 * |______________|
 *
 *
 * when it's pressed, a message appears:
 * ___________-[]X_
 * |              |
 * |  [Say hello] |
 * |    Hello!    |
 * |______________|
 *
 * that's it.
 */

class UIMultitouchTest: public testing::UITest {
public:
protected:

    class TestWindow: public AWindow {
    public:
        _<ASlider> mSlider1;
        _<ASlider> mSlider2;
        TestWindow() {
            using namespace declarative;
            setContents(Vertical {
                mSlider1 = _new<ASlider>(),
                mSlider2 = _new<ASlider>(),
            } with_style { MinSize { 200_dp, {} } });

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
 * Checks that single touch works correctly.
 */
TEST_F(UIMultitouchTest, Single) {
    By::value(mTestWindow->mSlider1).perform(pointerPress {
        .pointerIndex = APointerIndex::finger(0),
    });
    By::value(mTestWindow->mSlider1).perform(pointerRelease {
        .position = mTestWindow->mSlider1->getSize() - glm::ivec2(2, 2),
        .pointerIndex = APointerIndex::finger(0),
    });

    EXPECT_NEAR(mTestWindow->mSlider1->value(), 1.f, 0.1f);
}



/**
 * Checks that multi touch works correctly.
 */
TEST_F(UIMultitouchTest, Multi) {
    By::value(mTestWindow->mSlider1).perform(pointerPress {
        .pointerIndex = APointerIndex::finger(0),
    });
    By::value(mTestWindow->mSlider2).perform(pointerPress {
        .pointerIndex = APointerIndex::finger(1),
    });

    By::value(mTestWindow->mSlider1).perform(pointerMove {
        .position = mTestWindow->mSlider1->getSize() - glm::ivec2(2, 2),
        .pointerIndex = APointerIndex::finger(0),
    });
    By::value(mTestWindow->mSlider2).perform(pointerMove {
        .position = mTestWindow->mSlider2->getSize() / 2,
        .pointerIndex = APointerIndex::finger(1),
    });

    By::value(mTestWindow->mSlider1).perform(pointerRelease {
        .position = mTestWindow->mSlider1->getSize() - glm::ivec2(2, 2),
        .pointerIndex = APointerIndex::finger(0),
    });
    By::value(mTestWindow->mSlider2).perform(pointerRelease {
        .position = mTestWindow->mSlider2->getSize() / 2,
        .pointerIndex = APointerIndex::finger(1),
    });

    EXPECT_NEAR(mTestWindow->mSlider1->value(), 1.f, 0.1f);
    EXPECT_NEAR(mTestWindow->mSlider2->value(), 0.5f, 0.1f);
}
