/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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

class UIOpenGLRendererTest : public testing::Test {
public:

protected:
    void SetUp() override {
        Test::SetUp();

        ARenderingContextOptions::set({
          .initializationOrder {
            ARenderingContextOptions::OpenGL {},
          },
          .flags = ARenderContextFlags::NO_SMOOTH | ARenderContextFlags::NO_VSYNC,
        });

        try {
            mWindow = _new<AWindow>();
            mWindow->show();
        } catch (const AException& e) {
            do_once { ALogger::info("OpenGLRendererTest") << "GPU is not available; skipping test\n" << e; }
            GTEST_SKIP() << "GPU is not available";
        }
    }
    void TearDown() override { Test::TearDown(); }
    _<AWindow> mWindow;
};

TEST_F(UIOpenGLRendererTest, CheckRenderer) {
    EXPECT_TRUE(dynamic_cast<OpenGLRenderer*>(&AWindow::current()->getRenderingContext()->renderer()));
    mWindow->setContents(Centered {
      _new<AView>() with_style {
        BackgroundSolid { AColor::RED },
        FixedSize { 32_dp },
      } << ".test",
    });
    By::name(".test").check(averageColor(AColor::RED));
}