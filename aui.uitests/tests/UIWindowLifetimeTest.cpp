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
#include "AUI/Util/ALayoutInflater.h"
#include "AUI/View/ALabel.h"

using namespace declarative;

namespace {
class UIWindowLifetime : public testing::UITest {};
}   // namespace

// A window posts a task to the event loop from its constructor (see AWindow::windowNativePreInit), and nothing says
// the window is still alive when the event loop gets to it — a dialog may well be closed and released in between.
TEST_F(UIWindowLifetime, DestroyedBeforeItsQueuedTasksRun) {
    auto window = _new<AWindow>();
    ALayoutInflater::inflate(window, Label { "hello" });

    _weak<AWindow> observer = window;
    window = nullptr;
    ASSERT_TRUE(observer.expired()) << "the window must be gone for this test to test anything at all";

    // must not touch the window that is no more.
    AThread::processMessages();
}
