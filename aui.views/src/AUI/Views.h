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
#pragma warning(disable: 4251)

#include "AUI/api.h"
#include "AUI/Util/Factory.h"
#include "Util/IBackgroundEffect.h"


#if AUI_PLATFORM_WIN
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/**
 * @defgroup views aui::views
 * @brief Graphical User Interface library.
 * @details
 * AUI's flagman module which provides modern declarative ways to create graphical user interfaces.
 */

