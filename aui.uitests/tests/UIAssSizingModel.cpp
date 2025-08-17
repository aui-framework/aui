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
