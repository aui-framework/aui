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
#include "AUI/ASS/Property/MinSize.h"
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/AView.h"

using namespace declarative;

namespace {

class UIWindowMinSize : public testing::UITest {
protected:
    _<AWindow> mWindow;

    /**
     * @brief Creates the window; measuring its minimum size does not require it to be laid out or shown.
     */
    void build(_<AView> contents) {
        mWindow = _new<AWindow>();
        ALayoutInflater::inflate(mWindow, std::move(contents));
        mWindow->ensureAssUpdated();   // so that the window's own padding is readable by the tests below.
    }

    /**
     * @brief ...but observing what the window does with that minimum size does.
     */
    void show(_<AView> contents) {
        build(std::move(contents));
        mWindow->show();
        AUI_REPEAT(10) { uitest::frame(); }
    }

    void TearDown() override {
        mWindow = nullptr;
        UITest::TearDown();
    }
};

}   // namespace

// The window is never smaller than the space its contents need at their narrowest.
TEST_F(UIWindowMinSize, IsMeasuredFromContents) {
    build(Horizontal {
        _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 120_dp, 40_dp } },
        _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 80_dp, 30_dp } },
    });

    const auto padding = mWindow->getPadding();
    EXPECT_EQ(mWindow->getMinimumSize().x, 200 + padding.horizontal());
    EXPECT_EQ(mWindow->getMinimumSize().y, 40 + padding.vertical());
}

// Contents that need more than that are expected to scroll or to overflow: a window that does not fit on the screen
// is worse than a clipped view.
TEST_F(UIWindowMinSize, IsCapped) {
    build(Centered {
        _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 5000_dp } },
    });

    EXPECT_EQ(mWindow->getMinimumSize(), glm::ivec2(1000));
}

// An empty window demands nothing but its own padding.
TEST_F(UIWindowMinSize, IsOnlyPaddingWithoutContents) {
    build(nullptr);

    const auto padding = mWindow->getPadding();
    EXPECT_EQ(mWindow->getMinimumSize(), glm::ivec2(padding.horizontal(), padding.vertical()));
}

// A styled minimum of the window itself is respected as well.
TEST_F(UIWindowMinSize, RespectsStyledMinimumOfTheWindow) {
    build(Centered {
        _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 20_dp } },
    });
    mWindow->setCustomStyle({ ass::MinSize { 300_dp, 250_dp } });

    EXPECT_EQ(mWindow->getMinimumSize().x, 300);
    EXPECT_EQ(mWindow->getMinimumSize().y, 250);
}

// ...and the window is actually kept at that size.
TEST_F(UIWindowMinSize, WindowIsNotSizedBelowIt) {
    show(Centered {
        _new<AView>() AUI_OVERRIDE_STYLE { MinSize { 200_dp, 150_dp } },
    });

    // as if the user dragged the window smaller than its contents can be (see ASurface::onResize).
    mWindow->setSize({ 10, 10 });
    mWindow->requestLayout();
    mWindow->redraw();

    EXPECT_GE(mWindow->getSize().x, mWindow->getMinimumSize().x);
    EXPECT_GE(mWindow->getSize().y, mWindow->getMinimumSize().y);
    EXPECT_GT(mWindow->getMinimumSize().x, 0);
}
