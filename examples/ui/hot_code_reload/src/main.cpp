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

// [hardcoded_path]
#include <AUI/Platform/Entry.h>
#if __has_include(<AUI/Remote/AHotCodeReload.h>)
#include <AUI/Remote/AHotCodeReload.h>
#endif
#include "MyWindow.h"

AUI_ENTRY {
#if __has_include(<AUI/Remote/AHotCodeReload.h>)
    // hardcoded path
    AHotCodeReload::inst().addFile("/home/projects/aui/cmake-build-debug/CMakeFiles/aui.example.hot_code_reload.dir/src/MyWindow.cpp.o");
#endif

    _new<MyWindow>()->show();
    return 0;
}
// [hardcoded_path]
