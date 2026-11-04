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
#include "AUI/UITest.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Image/AImage.h"
#include "AUI/Platform/OpenGLRenderingContext.h"
#include "AUI/Util/UIBuildingHelpers.h"

using namespace ass;
using namespace declarative;

namespace {
constexpr unsigned INSET = 30;

glm::u8vec4 pixelAt(const AImage& image, glm::uvec2 position) {
    // the screenshot is RGBA, 8 bit per channel
    const auto* pixel = reinterpret_cast<const std::uint8_t*>(image.buffer().data()) + (position.y * image.width() + position.x) * 4;
    return { pixel[0], pixel[1], pixel[2], pixel[3] };
}

testing::AssertionResult isNear(glm::u8vec4 actual, glm::u8vec4 expected, int tolerance = 2) {
    for (int i = 0; i < 4; ++i) {
        if (std::abs(int(actual[i]) - int(expected[i])) > tolerance) {
            return testing::AssertionFailure() << "pixel is (" << int(actual.r) << ", " << int(actual.g) << ", "
                                               << int(actual.b) << ", " << int(actual.a) << "), expected ("
                                               << int(expected.r) << ", " << int(expected.g) << ", "
                                               << int(expected.b) << ", " << int(expected.a) << ")";
        }
    }
    return testing::AssertionSuccess();
}
}   // namespace

/**
 * Screenshots of a real OpenGL window. Opens a real window on the user's desktop, so it's opt-in.
 */
class UIOpenGLScreenshotTest : public testing::Test {
protected:
    void SetUp() override {
        Test::SetUp();
        if (!std::getenv("AUI_TEST_REAL_WINDOWS")) {
            GTEST_SKIP() << "Opens a real window; set AUI_TEST_REAL_WINDOWS=1 to run";
        }

        mPreviousOptions = ARenderingContextOptions::get();
        ARenderingContextOptions::set({
          .initializationOrder {
            ARenderingContextOptions::OpenGL {},
          },
          .flags = ARenderContextFlags::NO_SMOOTH | ARenderContextFlags::NO_VSYNC,
        });

        try {
            AWindow::setWindowManager<AWindowManager>();
            mWindow = _new<AWindow>("Screenshot test", 320_px, 200_px);
            mWindow->show();
        } catch (const AException& e) {
            TearDown();
            GTEST_SKIP() << "GPU is not available: " << e.getMessage();
        }
        if (!dynamic_cast<OpenGLRenderer*>(&mWindow->getRenderingContext()->renderer())) {
            TearDown();
            GTEST_SKIP() << "OpenGL renderer was not selected";
        }
    }

    void TearDown() override {
        mWindow = nullptr;
        AWindow::destroyWindowManager();
        if (mPreviousOptions) {
            ARenderingContextOptions::set(*mPreviousOptions);
            mPreviousOptions.reset();
        }
        Test::TearDown();
    }

    _<AWindow> mWindow;
    AOptional<ARenderingContextOptions> mPreviousOptions;
};

TEST_F(UIOpenGLScreenshotTest, HasWindowResolutionAndContent) {
    mWindow->setContents(Stacked {
      _new<AView>() AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::RED },
        Expanding(),
      },
    });
    mWindow->redraw();

    auto image = mWindow->getRenderingContext()->makeScreenshot();
    ASSERT_FALSE(image.buffer().empty());

    // the offscreen framebuffer is supersampled; the screenshot must be taken at the window's resolution.
    EXPECT_EQ(glm::ivec2(image.size()), mWindow->getSize());

    // the window keeps a margin around its contents; sample well inside of them.
    EXPECT_TRUE(isNear(pixelAt(image, { INSET, INSET }), { 255, 0, 0, 255 }));
    EXPECT_TRUE(isNear(pixelAt(image, image.size() / 2u), { 255, 0, 0, 255 }));
    EXPECT_TRUE(isNear(pixelAt(image, image.size() - (INSET + 1u)), { 255, 0, 0, 255 }));
}

TEST_F(UIOpenGLScreenshotTest, IsTopDown) {
    // red top half, blue bottom half
    mWindow->setContents(Vertical {
      _new<AView>() AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::RED },
        Expanding(),
      },
      _new<AView>() AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::BLUE },
        Expanding(),
      },
    });
    mWindow->redraw();

    auto image = mWindow->getRenderingContext()->makeScreenshot();
    ASSERT_FALSE(image.buffer().empty());
    EXPECT_TRUE(isNear(pixelAt(image, { image.width() / 2, INSET }), { 255, 0, 0, 255 }));
    EXPECT_TRUE(isNear(pixelAt(image, { image.width() / 2, image.height() - INSET - 1 }), { 0, 0, 255, 255 }));
}

TEST_F(UIOpenGLScreenshotTest, DoesNotDisturbRendering) {
    mWindow->setContents(Stacked {
      _new<AView>() AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::GREEN },
        Expanding(),
      },
    });
    mWindow->redraw();
    auto first = mWindow->getRenderingContext()->makeScreenshot();
    mWindow->redraw();
    auto second = mWindow->getRenderingContext()->makeScreenshot();
    ASSERT_FALSE(first.buffer().empty());
    EXPECT_EQ(first.size(), second.size());
    EXPECT_TRUE(isNear(pixelAt(second, second.size() / 2u), { 0, 255, 0, 255 }));
}
