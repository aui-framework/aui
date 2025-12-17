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
// Created by Nelonn on 12/17/2025.
//

// Credit: SDL3

#include "Theme.h"

#include <dwmapi.h>

typedef enum {
    UXTHEME_APPMODE_DEFAULT,
    UXTHEME_APPMODE_ALLOW_DARK,
    UXTHEME_APPMODE_FORCE_DARK,
    UXTHEME_APPMODE_FORCE_LIGHT,
    UXTHEME_APPMODE_MAX
} UxthemePreferredAppMode;

typedef enum {
    WCA_UNDEFINED = 0,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
} WINDOWCOMPOSITIONATTRIB;

typedef struct {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef struct {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR szCSDVersion[128];
} NT_OSVERSIONINFOW;

typedef bool (WINAPI *ShouldAppsUseDarkMode_t)(void);
typedef void (WINAPI *AllowDarkModeForWindow_t)(HWND, bool);
typedef void (WINAPI *AllowDarkModeForApp_t)(bool);
typedef void (WINAPI *RefreshImmersiveColorPolicyState_t)(void);
typedef UxthemePreferredAppMode (WINAPI *SetPreferredAppMode_t)(UxthemePreferredAppMode);
typedef BOOL (WINAPI *SetWindowCompositionAttribute_t)(HWND, const WINDOWCOMPOSITIONATTRIBDATA *);
typedef void (NTAPI *RtlGetVersion_t)(NT_OSVERSIONINFOW *);

namespace aui {

bool IsWindowsBuildVersionAtLeast(DWORD dwBuildNumber) {
    static DWORD BuildNumber = 0;
    if (BuildNumber != 0) {
        return (BuildNumber >= dwBuildNumber);
    }

    HMODULE ntdll = LoadLibrary(TEXT("ntdll.dll"));
    if (!ntdll) {
        return false;
    }
    // There is no function to get Windows build number, so let's get it here via RtlGetVersion
    RtlGetVersion_t RtlGetVersionFunc = (RtlGetVersion_t)GetProcAddress(ntdll, "RtlGetVersion");
    NT_OSVERSIONINFOW os_info;
    os_info.dwOSVersionInfoSize = sizeof(NT_OSVERSIONINFOW);
    os_info.dwBuildNumber = 0;
    if (RtlGetVersionFunc) {
        RtlGetVersionFunc(&os_info);
    }
    FreeLibrary(ntdll);

    BuildNumber = (os_info.dwBuildNumber & ~0xF0000000);
    return (BuildNumber >= dwBuildNumber);
}

void UpdateDarkModeForHWND(HWND hwnd) {
    if (!IsWindowsBuildVersionAtLeast(17763)) return;

    HMODULE uxtheme = LoadLibrary(TEXT("uxtheme.dll"));
    if (!uxtheme) return;

    RefreshImmersiveColorPolicyState_t RefreshImmersiveColorPolicyStateFunc = (RefreshImmersiveColorPolicyState_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(104));
    ShouldAppsUseDarkMode_t ShouldAppsUseDarkModeFunc = (ShouldAppsUseDarkMode_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(132));
    AllowDarkModeForWindow_t AllowDarkModeForWindowFunc = (AllowDarkModeForWindow_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(133));
    if (!IsWindowsBuildVersionAtLeast(18362)) {
        AllowDarkModeForApp_t AllowDarkModeForAppFunc = (AllowDarkModeForApp_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(135));
        if (AllowDarkModeForAppFunc) {
            AllowDarkModeForAppFunc(true);
        }
    } else {
        SetPreferredAppMode_t SetPreferredAppModeFunc = (SetPreferredAppMode_t)GetProcAddress(uxtheme, MAKEINTRESOURCEA(135));
        if (SetPreferredAppModeFunc) {
            SetPreferredAppModeFunc(UXTHEME_APPMODE_ALLOW_DARK);
        }
    }
    if (RefreshImmersiveColorPolicyStateFunc) {
        RefreshImmersiveColorPolicyStateFunc();
    }
    if (AllowDarkModeForWindowFunc) {
        AllowDarkModeForWindowFunc(hwnd, true);
    }
    BOOL value;
    if (ShouldAppsUseDarkModeFunc) {
        value = ShouldAppsUseDarkModeFunc() ? TRUE : FALSE;
    } else {
        value = TRUE;
    }
    FreeLibrary(uxtheme);
    if (!IsWindowsBuildVersionAtLeast(18362)) {
        SetProp(hwnd, TEXT("UseImmersiveDarkModeColors"), reinterpret_cast<HANDLE>(static_cast<INT_PTR>(value)));
    } else {
        HMODULE user32 = GetModuleHandle(TEXT("user32.dll"));
        if (user32) {
            SetWindowCompositionAttribute_t SetWindowCompositionAttributeFunc = (SetWindowCompositionAttribute_t)GetProcAddress(user32, "SetWindowCompositionAttribute");
            if (SetWindowCompositionAttributeFunc) {
                WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &value, sizeof(value) };
                SetWindowCompositionAttributeFunc(hwnd, &data);
            }
        }
    }
}

}
