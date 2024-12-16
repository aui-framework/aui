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

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Url/AUrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif AUI_PLATFORM_LINUX
#include <gtk/gtk.h>

namespace aui::detail {
extern int argc;
extern char** argv;
}
#endif

/**
 * @defgroup views aui::views
 * @brief Graphical User Interface library.
 * @details AUI's flagship module which provides modern declarative ways to create graphical user interfaces.
 */



struct AViewsInit
{
    AViewsInit() {
#if AUI_PLATFORM_WIN
#ifndef AUI_NO_HIDPI
        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);
        auto SetProcessDpiAwarenessContext = (SetProcessDpiAwarenessContext_t)GetProcAddress(GetModuleHandleA("User32.dll"), "SetProcessDpiAwarenessContext");

        if (SetProcessDpiAwarenessContext) {
            // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
            SetProcessDpiAwarenessContext((HANDLE) -4);
        }
#endif

#elif AUI_PLATFORM_LINUX
    gtk_init(&aui::detail::argc, &aui::detail::argv);
#endif
    }
} init;