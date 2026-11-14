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

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>

using namespace declarative;

/**
 * A touch that comes while a flung scroll area is still gliding stops it, whatever its pointer index: iOS Safari gives
 * every touch a new Touch.identifier, so the next touch never had the index of the finger that flung.
 */
class UIKineticScrollTest : public testing::UITest {
protected:
    void SetUp() override {
        UITest::SetUp();
        mWindow = _new<AWindow>("Test window", 200_dp, 200_dp);
        auto rows = AVector<_<AView>>();
        for (int i = 0; i < 50; ++i) {
            rows << (Label { "Row" } AUI_OVERRIDE_STYLE { FixedSize { 150_dp, 40_dp } });
        }
        mWindow->setContents(Vertical {
          mScrollArea = AScrollArea::Builder().withContents(Vertical { std::move(rows) }).build()
              AUI_OVERRIDE_STYLE { FixedSize { 150_dp, 200_dp } },
        });
        mWindow->show();
        uitest::frame();
    }

    void TearDown() override {
        mWindow = nullptr;
        mScrollArea = nullptr;
        UITest::TearDown();
    }

    /// Drags the content up with finger 0 and lets it go, so it keeps scrolling by inertia.
    void fling() {
        const auto finger = APointerIndex::finger(0);
        glm::vec2 pos { 75_dp, 180_dp };
        mWindow->onPointerPressed({ .position = pos, .pointerIndex = finger });
        for (int i = 0; i < 4; ++i) {
            pos.y -= 20_dp;
            mWindow->onPointerMove(pos, { .pointerIndex = finger });
        }
        mWindow->onPointerReleased({ .position = pos, .pointerIndex = finger });
    }

    void frames(int count) {
        for (int i = 0; i < count; ++i) {
            mWindow->flagRedraw();
            uitest::frame();
        }
    }

    _<AWindow> mWindow;
    _<AScrollArea> mScrollArea;
};

TEST_F(UIKineticScrollTest, FlingKeepsScrolling) {
    fling();
    const auto released = mScrollArea->scroll().y;
    frames(5);
    EXPECT_GT(mScrollArea->scroll().y, released) << "no inertia: the test would prove nothing";
}

TEST_F(UIKineticScrollTest, NextTouchStopsInertia) {
    fling();
    frames(3);
    const auto touch = APointerIndex::finger(1);
    const glm::vec2 pos { 75_dp, 100_dp };
    mWindow->onPointerPressed({ .position = pos, .pointerIndex = touch });
    const auto stopped = mScrollArea->scroll().y;
    frames(10);
    EXPECT_EQ(mScrollArea->scroll().y, stopped) << "the scroll area keeps gliding under the finger";
    mWindow->onPointerReleased({ .position = pos, .pointerIndex = touch });
}
