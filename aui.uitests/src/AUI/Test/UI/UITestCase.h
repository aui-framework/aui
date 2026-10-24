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

//
// Created by Alex2772 on 12/18/2021.
//

#pragma once


#include <gtest/gtest.h>

#include <AUI/Platform/AWindow.h>
#include <AUI/Render/ARender/Software/SoftwareRenderer.h>


namespace testing {
    /**
     * @brief Base class of UI test suites.
     * @ingroup uitests
     * @details
     * Derive your suite from UITest and use `TEST_F`. Before each test, UITest replaces the window manager with a stub
     * one (windows are not shown on the screen, rendering goes to a software renderer). After each test, it closes all
     * windows and destroys the window manager.
     *
     * ```cpp
     * class UIMyWindow : public testing::UITest {
     * protected:
     *     void SetUp() override {
     *         UITest::SetUp();   // do not forget to call the base
     *         _new<MyWindow>()->show();
     *     }
     * };
     *
     * TEST_F(UIMyWindow, Hello) {
     *     By::text("Say hello").perform(click());
     * }
     * ```
     *
     * !!! warning
     *
     *     If you override `SetUp` or `TearDown`, call `UITest::SetUp()` first and `UITest::TearDown()` last.
     *     Release every window and view you own before calling `UITest::TearDown()`.
     *
     * Every failed assertion made via `UIMatcher` saves a screenshot to `reports/<suite>/fail-<test>.png` with the
     * matched views outlined in red; a screenshot of the final state of every test is saved as
     * `reports/<suite>/finish-<test>.png`.
     *
     * For views to update (layout, styles, pending messages), the UI must be processed; actions and checks of
     * `UIMatcher` do that for you. In other cases, call `uitest::frame()`.
     */
    class API_AUI_UITESTS UITest : public testing::Test {
    public:
        ~UITest() override = default;

    protected:
        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Saves a screenshot of the current state to `reports/` (useful for inspecting a test by eye).
         * @param name suffix of the file name; the file is named `<suite>.<test>_<name>.png`.
         */
        void saveScreenshot(const AString& name);

    public:

    };
}