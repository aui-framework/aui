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

class UIAssSizingModel : public testing::UITest {
public:
    void SetUp() override {
        UITest::SetUp();

        mTestWindow = _new<AWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        mTestWindow = nullptr;
        UITest::TearDown();
    }

    _<AWindow> mTestWindow;
};

// AUI_DOCS_OUTPUT: doxygen/intermediate/ass_sizing_model.h
// @page "AUI Box Model"
//
// The AUI Box Model is layout structure for all types of AViews, which combines margin, padding, border, and background
// elements to create a visual container, defining the outer appearance of each AView as a rectangular block on-screen.
// ## Box key components
//
// - **View's content box**: the area are whose size is defined by [AView::getContentMinimumSize()] and
//   [AView::getContentSize()]. The content area is the innermost area, wrapping view's specific contents only (i.e,
//   label's text); control it by implementing [AView::getContentMinimumWidth()] and
//   [AView::getContentMinimumHeight()].
// - **View's box**: the area whose size is defined by AView::size(). Use [ass::FixedSize], [ass::MinSize] and
//   [ass::MaxSize] to define an acceptable size range. Use [ass::Padding] to add a blank space between view's box
//   and view's context box.
//
//   A view is responsible to handle its padding properly. As an AUI user, you wouldn't need to bother about that unless
//   you are implementing custom [AView::render()].
//
//   [BackgroundSolid](ass::BackgroundSolid) and similar [ass] properties are fit into this box.
//
//   [Border](ass::Border) property outlines this box from the inner side, and never affects the layout; it's just a
//   visual trait.
//
// - **View's margin**: the margin is the outermost layer, wrapping the content, padding, and border as whitespace between
//   this box and other elements; control it using [ass::Margin].
//
//   Layout manager of view's parent is responsible to handling margin properly. All
//   [layout managers](layout-managers) tend to honor children margins; with an exception to [AWindow] (it's
//   margin has no effect) and [AAbsoluteLayout] (positioning and sizing is defined manually). As an AUI user, you
//   wouldn't need to bother about that unless you are implementing custom layout manager.
//

TEST_F(UIAssSizingModel, AllBoxesAreIdential) {
    // In this example, the equally sized boxes are produced with `FixedSize`, `MinSize` and `MaxSize`, respectively.
    // `FixedSize` will always acquire the specified size, `MinSize` specifies the minimum size that can potentially
    // grow further, and `MaxSize` restricts [expanding](EXPANDING) by specified size.
    // AUI_DOCS_CODE_BEGIN
    using namespace declarative;
    using namespace ass;

    static auto testView = [] {
        return Centered {
            Label { "all boxes should be equal" } AUI_WITH_STYLE {
                  Border { 1_px, AColor::BLACK },
                  Margin { 0 },
                  Padding { 0 },
                  Expanding {},
                },
        } << ".test";
    };

    mTestWindow->setContents(Vertical {
      Centered {
        testView() AUI_WITH_STYLE { Padding { 10_dp }, Border { 1_px, AColor::BLACK }, FixedSize { 200_dp, {} } },
      },
      Centered {
        testView() AUI_WITH_STYLE { Padding { 10_dp }, Border { 1_px, AColor::BLACK }, MinSize { 200_dp, {} } },
      },
      Centered {
        testView() AUI_WITH_STYLE { Padding { 10_dp }, Border { 1_px, AColor::BLACK }, MaxSize { 200_dp, {} }, Expanding {} },
      },
    });
    // AUI_DOCS_CODE_END
    auto selector = By::name(".test");
    selector.check(sameWidth(), "width is not the same").check(sameHeight(), "height is not the same");
    for (const auto& view : selector.toVector()) {
        EXPECT_EQ(view->getSize().x, 200);
    }

    // ![](imgs/UIAssSizingModel.AllBoxesAreIdential_1.png)
    saveScreenshot("1");
}
//
// # Comparison to CSS
//
// AUI shares principles with [web technologies](https://developer.mozilla.org/en-US/docs/Learn_web_development/Core/Styling_basics/Box_model#what_is_the_css_box_model)
// with some exceptions. In AUI:
//
// - border never affects layout
// - element's size includes padding, thus it differs from "content area"
//
// AUI's box model is equal to CSS's ["alternate box model"](https://developer.mozilla.org/en-US/docs/Learn_web_development/Core/Styling_basics/Box_model#playing_with_box_models),
// with an exception to border.
