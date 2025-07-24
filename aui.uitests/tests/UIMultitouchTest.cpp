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

class SliderMock: public ASlider {
public:
    MOCK_METHOD(void, onPointerMove, (glm::vec2 pos, const APointerMoveEvent& e), (override));
};

class UIMultitouchTest: public testing::UITest {
public:
protected:

    class TestWindow: public AWindow {
    public:
        _<ASlider> mSlider1;
        _<SliderMock> mSlider2;
        TestWindow() {
            using namespace declarative;
            setContents(Vertical {
                mSlider1 = _new<ASlider>(),
                mSlider2 = _new<SliderMock>(),
            } AUI_WITH_STYLE { MinSize { 200_dp, {} } });

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

    EXPECT_NEAR(*mTestWindow->mSlider1->value(), 1.f, 0.1f);
}



/**
 * Checks that multi touch works correctly.
 */
TEST_F(UIMultitouchTest, Multi) {
    auto matcher = [&](glm::vec2 v) {
        return glm::distance(glm::vec2(mTestWindow->mSlider2->getSize()) / 2.f, v) <= 20.f;
    };
    EXPECT_CALL(*mTestWindow->mSlider2, onPointerMove(testing::Truly(matcher), testing::_)).Times(testing::AtLeast(1));

    ON_CALL(*mTestWindow->mSlider2, onPointerMove).WillByDefault([&](const auto&... a) {
        mTestWindow->mSlider2->ASlider::onPointerMove(a...);
    });

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

    EXPECT_NEAR(*mTestWindow->mSlider1->value(), 1.f, 0.1f);
    EXPECT_NEAR(*mTestWindow->mSlider2->value(), 0.5f, 0.1f);
}
