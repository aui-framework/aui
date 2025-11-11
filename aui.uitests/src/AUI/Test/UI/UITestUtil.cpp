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
// Created by alex2772 on 12/21/21.
//

#include "UITestUtil.h"

#include "AUI/UITestState.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AStubWindowManager.h"

void uitest::setup() {
    UITestState::beginUITest();
    AWindow::setWindowManager<AStubWindowManager>();
}

void uitest::frame() {
    AStubWindowManager::drawFrame();
    AThread::processMessages();
}
