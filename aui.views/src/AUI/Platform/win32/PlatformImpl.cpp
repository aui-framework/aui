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

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>

#include <Windows.h>

float APlatform::getDpiRatio()
{
    typedef UINT(WINAPI *GetDpiForSystem_t)();
    static auto GetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForSystem");
    if (GetDpiForSystem) {
        return GetDpiForSystem() / 96.f;
    }
    return 1.f;
}

void APlatform::openUrl(const AUrl& url) {
    auto wurl = aui::win32::toWchar(url.full());
    ShellExecute(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_NORMAL);
}
