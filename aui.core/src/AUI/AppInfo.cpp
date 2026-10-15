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
// Created by mikle on 23.07.25.
//

#include "AppInfo.h"

#if AUI_COMPILER_MSVC || defined(_MSC_VER)
// make sure these defaults are initialized BEFORE the dynamic initializers of the user's code (appinfo_*.cpp generated
// by aui_app); otherwise, in static builds, they would overwrite values set by aui_app depending on the link order.
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#endif

// weak data - will be overridden by aui_app if linked together, otherwise defaults to these values
AString aui::app_info::name = "unknown";
AString aui::app_info::app_id = "com.unknown.aui-application";