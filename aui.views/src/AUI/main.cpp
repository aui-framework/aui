// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Url/AUrl.h>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
#endif
    }
} init;