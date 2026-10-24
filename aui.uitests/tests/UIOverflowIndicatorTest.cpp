// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/UITest.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ALabel.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/AScrollAreaViewport.h>
#include <AUI/View/AView.h>

using namespace ass;
using namespace declarative;

/**
 * @brief Tests ASurface::Profiling::showOverflow (devtools' "Show overflow").
 */
class UIOverflowIndicator : public testing::UITest {
protected:
    void SetUp() override {
        UITest::SetUp();

        mWindow = _new<AWindow>("", 400_dp, 200_dp);
        mWindow->setContents(Horizontal {
          Stacked {
            _new<AView>() AUI_OVERRIDE_STYLE {
              FixedSize { 200_dp },
              BackgroundSolid { AColor::WHITE },
            },
          } << ".clipping" AUI_OVERRIDE_STYLE {
            FixedSize { 100_dp },
            BackgroundSolid { AColor::WHITE },
            AOverflow::HIDDEN,
          },
          _new<AView>() << ".plain" AUI_OVERRIDE_STYLE {
            FixedSize { 50_dp },
            BackgroundSolid { AColor::WHITE },
            Margin { 8_dp },
          },
          Horizontal {
            Label { "a label that is way too long to fit here" } << ".squeezed" AUI_OVERRIDE_STYLE {
              AOverflow::HIDDEN,
            },
          } AUI_OVERRIDE_STYLE { FixedSize { 60_dp, 20_dp } },
          AScrollArea::Builder()
              .withContents(Vertical {
                _new<AView>() AUI_OVERRIDE_STYLE {
                  FixedSize { 40_dp, 400_dp },
                  BackgroundSolid { AColor::WHITE },
                },
              })
              .build() AUI_LET { it->setFixedSize({ 60_dp, 60_dp }); },
        });
        mWindow->show();
    }

    void TearDown() override { UITest::TearDown(); }

    static void showOverflow(bool value) {
        AWindow::current()->profiling()->showOverflow = value;
        AWindow::current()->redraw();
    }

    _<AWindow> mWindow;
};

TEST_F(UIOverflowIndicator, DisabledByDefault) {
    By::name(".clipping").check(averageColor(AColor::WHITE), "clipping view is marked despite showOverflow is off");
}

TEST_F(UIOverflowIndicator, MarksCutOffEdges) {
    showOverflow(true);

    auto screenshot = ScreenshotAnalyzer::makeScreenshot().clip(By::name(".clipping").one());
    const auto& image = screenshot.image();

    // contents of the clipping view stick out of it on every side, so every edge is marked.
    for (auto position : {
           glm::uvec2 { 1, image.height() / 2 },
           glm::uvec2 { image.width() / 2, 1 },
           glm::uvec2 { image.width() - 2, image.height() / 2 },
           glm::uvec2 { image.width() / 2, image.height() - 2 },
         }) {
        auto pixel = image.get(position);
        EXPECT_GT(pixel.r, pixel.b + 0.2f) << "edge at " << position.x << ";" << position.y << " is not marked";
    }

    auto center = image.get(image.size() / 2u);
    EXPECT_NEAR(center.r, center.b, 0.05f) << "contents of the clipping view are painted over";
}

/**
 * @brief Contents of a plain view are observed as well, i.e. a text squeezed by its parent.
 */
TEST_F(UIOverflowIndicator, MarksCutOffText) {
    showOverflow(true);

    auto screenshot = ScreenshotAnalyzer::makeScreenshot().clip(By::name(".squeezed").one());
    const auto& image = screenshot.image();

    auto pixel = image.get({ image.width() - 2, image.height() / 2 });
    EXPECT_GT(pixel.r, pixel.b + 0.2f) << "the edge the text is cut off by is not marked";
}

/**
 * @brief A scroll area is expected to have more contents than it displays at once, so it's not marked.
 */
TEST_F(UIOverflowIndicator, LeavesScrollAreasIntact) {
    showOverflow(true);

    auto screenshot = ScreenshotAnalyzer::makeScreenshot().clip(By::type<AScrollAreaViewport>().one());
    const auto& image = screenshot.image();

    for (auto position : {
           glm::uvec2 { 1, image.height() / 2 },
           glm::uvec2 { image.width() / 2, 1 },
           glm::uvec2 { image.width() - 2, image.height() / 2 },
           glm::uvec2 { image.width() / 2, image.height() - 2 },
         }) {
        auto pixel = image.get(position);
        EXPECT_NEAR(pixel.r, pixel.b, 0.05f) << "edge at " << position.x << ";" << position.y << " is marked";
    }
}

TEST_F(UIOverflowIndicator, LeavesVisibleOverflowViewsIntact) {
    showOverflow(true);

    By::name(".plain").check(averageColor(AColor::WHITE), "view with AOverflow::VISIBLE is marked");
}
