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
// Created by alex2772 on 10/6/25.
//

#include "AHotCodeReload.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/kAUI.h"
#include "AUI/Platform/AProcess.h"

namespace {

static constexpr auto LOG_TAG = "Hot code reload";

}

AHotCodeReload::AHotCodeReload():
  mLoader(AProcess::self()->getPathToExecutable())
{

}

AHotCodeReload::~AHotCodeReload() = default;

void AHotCodeReload::reload() {
    AThread::current()->enqueue([this] {
        AString input =
            "/home/alex2772/CLionProjects/aui/cmake-build-debug/examples/ui/hot_code_reload/CMakeFiles/"
            "aui.example.hot_code_reload.dir/src/main.cpp.o";
        emit patchBegin;
        mLoader.load(input);
        AUI_DEFER { emit patchEnd; };
        ALogger::info(LOG_TAG) << "Binary reloaded: " << input;
    });
}


