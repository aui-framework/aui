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

#pragma once

/**
 * @defgroup uitests aui::uitests
 * @brief UI testing: find views, simulate user input and check what is on the screen.
 * @details
 * aui::uitests runs your real windows and views inside a GTest test, without a native window: a stub window manager and
 * a software renderer are used, so tests are fast and work on a headless CI. A test finds views with `By`, acts on them
 * as a user would (click, type, scroll) and checks the result.
 *
 * ```cpp
 * #include <AUI/UITest.h>
 *
 * class UIGreeting : public testing::UITest {};
 *
 * TEST_F(UIGreeting, Click) {
 *     _new<MyWindow>()->show();
 *     By::text("Say hello").perform(click());
 *     By::name("#hello").check(visible()).check(text("Hello!"));
 * }
 * ```
 *
 * Include `<AUI/UITest.h>` to get everything. If your project uses `aui_enable_tests` and AUI was imported with the
 * `uitests` component (`auib_import(aui ... COMPONENTS core views uitests)`), `aui::uitests` is linked to the `Tests`
 * target automatically; see [writing-tests] for the general test setup.
 *
 * Pages of this module:
 *
 * - [uitests-first-test] - fixture, window, a complete test;
 * - [uitests-selecting-views] - finding views with `By` and combining matchers;
 * - [uitests-actions] - clicks, typing, key presses, scrolling;
 * - [uitests-assertions] - built-in checks, layout checks and custom assertions;
 * - [uitests-screenshots] - pixel checks and failure reports.
 *
 * `testing::UITest` is the base class of UI test suites, `UIMatcher` is the object returned by `By`.
 */

#include <AUI/View/AView.h>

#include <AUI/Test/UI/UITestUtil.h>

#include <gtest/gtest.h>
#include <AUI/Test/UI/UIMatcher.h>
#include <AUI/Test/UI/By.h>
#include <AUI/Test/UI/UITestCase.h>

#include <AUI/Test/UI/Action/Click.h>
#include <AUI/Test/UI/Action/Gesture.h>
#include <AUI/Test/UI/Action/DoubleClick.h>
#include <AUI/Test/UI/Action/Type.h>
#include <AUI/Test/UI/Action/MouseMove.h>
#include <AUI/Test/UI/Action/Scroll.h>
#include <AUI/Test/UI/Action/KeyDownAndUp.h>

#include <AUI/Test/UI/Assertion/Align.h>
#include <AUI/Test/UI/Assertion/Empty.h>
#include <AUI/Test/UI/Assertion/Gone.h>
#include <AUI/Test/UI/Assertion/Visible.h>
#include <AUI/Test/UI/Assertion/NotVisible.h>
#include <AUI/Test/UI/Assertion/Text.h>
#include <AUI/Test/UI/Assertion/Size.h>
#include <AUI/Test/UI/Assertion/Color.h>
#include <AUI/Test/UI/Assertion/ParentOf.h>
