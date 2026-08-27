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
#include <AUI/View/AView.h>
#include <AUI/Platform/AWindow.h>

using namespace declarative;

class UIFocusNavigation : public testing::UITest {
protected:
    /**
     * @brief Minimal view that participates in keyboard (non-mouse) focus navigation.
     * @details
     * collectFocusableViews requires both capturesFocus() and handlesNonMouseNavigation() to be true.
     * AView::capturesFocus() returns true by default, so only handlesNonMouseNavigation() has to be overridden.
     */
    struct FocusableView : AView {
        bool handlesNonMouseNavigation() override { return true; }
    };

    _<AWindow> mWindow;
    _<FocusableView> v1, v2, v3;

    void SetUp() override {
        UITest::SetUp();

        v1 = _new<FocusableView>();
        v2 = _new<FocusableView>();
        v3 = _new<FocusableView>();

        mWindow = _new<AWindow>();
        mWindow->setContents(Vertical {
            v1,
            v2,
            v3,
        });
        mWindow->show();

        // process attach/layout messages so that getWindow()/focus() works
        uitest::frame();
    }

    /**
     * @brief Currently focused view as a raw pointer.
     */
    [[nodiscard]] AView* focused() const {
        return mWindow->getFocusedView().get();
    }

    /**
     * @brief Pumps UI messages so that focus() (dispatched to the UI thread) takes effect.
     */
    static void pump() {
        uitest::frame();
    }
};

/**
 * With no view focused, focusNextView() should focus the first focusable view.
 */
TEST_F(UIFocusNavigation, NoFocusFocusesFirst) {
    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusNextView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * With no view focused, focusPrevView() should focus the last focusable view.
 */
TEST_F(UIFocusNavigation, NoFocusFocusesLast) {
    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusPrevView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * focusNextView() moves focus to the next focusable view in order.
 */
TEST_F(UIFocusNavigation, SequentialNext) {
    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * focusPrevView() moves focus to the previous focusable view in order.
 */
TEST_F(UIFocusNavigation, SequentialPrev) {
    v3->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v3.get()));

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * focusNextView() wraps around from the last focusable view back to the first.
 */
TEST_F(UIFocusNavigation, WrapAroundNext) {
    v3->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v3.get()));

    mWindow->focusNextView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * focusPrevView() wraps around from the first focusable view back to the last.
 */
TEST_F(UIFocusNavigation, WrapAroundPrev) {
    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusPrevView();
    pump();

    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * Disabled views must be skipped by the focus traversal.
 */
TEST_F(UIFocusNavigation, SkipsDisabled) {
    v2->setEnabled(false);

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    // v2 is disabled, so next after v1 is v3
    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));

    // wraps back to v1
    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));

    // from v1, prev wraps to v3 (v2 is disabled)
    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * Invisible (INVISIBLE or GONE) views must be skipped by the focus traversal.
 */
TEST_F(UIFocusNavigation, SkipsInvisible) {
    v2->setVisibility(Visibility::INVISIBLE);

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));

    // GONE must also be skipped
    v2->setVisibility(Visibility::GONE);
    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v1.get()));
}

/**
 * Focus traversal must visit nested views in depth-first order and skip non-navigable containers.
 */
TEST_F(UIFocusNavigation, DepthFirstOrder) {
    auto nested1 = _new<FocusableView>();
    auto nested2 = _new<FocusableView>();

    mWindow->setContents(Vertical {
        v1,
        Vertical {
            nested1,
            nested2,
        },
        v2,
        v3,
    });
    uitest::frame();

    v1->focus();
    pump();
    ASSERT_EQ(focused(), static_cast<AView*>(v1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(nested1.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(nested2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v2.get()));

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), static_cast<AView*>(v3.get()));
}

/**
 * When there are no focusable views, focusNextView()/focusPrevView() must be no-ops and keep focus unset.
 */
TEST_F(UIFocusNavigation, NoFocusableViews) {
    v1->setEnabled(false);
    v2->setEnabled(false);
    v3->setEnabled(false);

    mWindow->setFocusedView(nullptr);
    pump();
    ASSERT_EQ(focused(), nullptr);

    mWindow->focusNextView();
    pump();
    EXPECT_EQ(focused(), nullptr);

    mWindow->focusPrevView();
    pump();
    EXPECT_EQ(focused(), nullptr);
}
