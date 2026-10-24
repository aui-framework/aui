/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <AUI/UITest.h>
#include <AUI/UITestState.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>

using namespace declarative;

/**
 * UI tests run with system dpi 1 unless told otherwise; these check the dpi pipeline on a simulated hidpi screen.
 */
class UIDpiTest : public testing::UITest {
protected:
    _<AWindow> makeWindow() {
        auto window = _new<AWindow>("dpi", 400_px, 300_px);
        window->setContents(Centered { _new<AView>() << ".box" });
        window->show();
        return window;
    }
};

TEST_F(UIDpiTest, DefaultIsOne) {
    EXPECT_FLOAT_EQ(makeWindow()->getDpiRatio(), 1.f);
}

TEST_F(UIDpiTest, SystemDpiIsApplied) {
    UITestState::setSystemDpi(2.f);
    EXPECT_FLOAT_EQ(makeWindow()->getDpiRatio(), 2.f);
}

TEST_F(UIDpiTest, ScalingFactorMultipliesSystemDpi) {
    UITestState::setSystemDpi(2.f);
    auto window = makeWindow();
    window->setScalingParams({ .scalingFactor = 1.5f });
    EXPECT_FLOAT_EQ(window->getDpiRatio(), 3.f);
}

TEST_F(UIDpiTest, DpMetricResolvesWithSurfaceDpi) {
    UITestState::setSystemDpi(2.f);
    auto window = makeWindow();
    window->makeCurrent();
    EXPECT_FLOAT_EQ(AMetric(10_dp).getValuePx(), 20.f);
}
