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
#include "AUI/Util/UIBuildingHelpers.h"

using namespace ass;
using namespace declarative;

class UIOpenGLRendererTest : public testing::Test {
public:

protected:
    void SetUp() override {
        Test::SetUp();

        if (std::getenv("CI")) {
            GTEST_SKIP() << "Disabled on CI";
        }

        ARenderingContextOptions::set({
          .initializationOrder {
            ARenderingContextOptions::OpenGL {},
          },
          .flags = ARenderContextFlags::NO_SMOOTH | ARenderContextFlags::NO_VSYNC,
        });

        try {
            AWindow::setWindowManager<AWindowManager>();
            mWindow = _new<AWindow>();
            mWindow->show();
        } catch (const AException& e) {
            AUI_DO_ONCE { ALogger::info("OpenGLRendererTest") << "GPU is not available; skipping test\n" << e; }
            GTEST_SKIP() << "GPU is not available";
        }
    }
    void TearDown() override { Test::TearDown(); }
    _<AWindow> mWindow;
};

#include <iostream>
#include <AUI/Image/png/PngImageLoader.h>
#include <AUI/IO/AFileOutputStream.h>

TEST_F(UIOpenGLRendererTest, CheckRenderer) {
    EXPECT_TRUE(dynamic_cast<OpenGLRenderer*>(&AWindow::current()->getRenderingContext()->backend()));
    mWindow->setContents(Centered {
      _new<AView>() << ".test" AUI_OVERRIDE_STYLE {
        BackgroundSolid { AColor::RED },
        FixedSize { 32_dp },
      },
    });
    auto view = By::name(".test").one();
    auto analyzer = ScreenshotAnalyzer::makeScreenshot();
    auto img = analyzer.image();
    std::cout << "DEBUG: img size: " << img.width() << "x" << img.height() << std::endl;
    if (view) {
        auto clipAnalyzer = analyzer.clip(view);
        auto avg = clipAnalyzer.averageColor();
        std::cout << "DEBUG: clip size: " << clipAnalyzer.image().width() << "x" << clipAnalyzer.image().height() << std::endl;
        std::cout << "DEBUG: avg color: " << avg.r << ", " << avg.g << ", " << avg.b << ", " << avg.a << std::endl;
    }
    if (!img.buffer().empty()) {
        AFileOutputStream fos("debug_checkrenderer.png");
        PngImageLoader::save(fos, img);
    }
    By::name(".test").check(averageColor(AColor::RED));
}
