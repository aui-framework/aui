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
// Created by alex2 on 26.11.2020.
//

#include "AUI/Platform/AClipboard.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/AWindowManager.h"

#include <Windows.h>

void AClipboard::copyToClipboard(const AString& text) {
    const size_t len = text.length() * 2 + 2;
    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), text.data(), len);
    GlobalUnlock(hMem);
    OpenClipboard(nullptr);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
}

AString AClipboard::pasteFromClipboard() {
    OpenClipboard(nullptr);
    HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
    std::wstring_view memView = (const wchar_t*)GlobalLock(hMem);
    AUI_DEFER {
        GlobalUnlock(hMem);
        CloseClipboard();
    };

    if (memView.data()) {
        AString s(reinterpret_cast<const char16_t*>(memView.data()), memView.length());

        return s;
    }
    return {};
}

bool AClipboard::isEmpty() {
    OpenClipboard(nullptr);
    HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
    auto azaza = (const wchar_t*)GlobalLock(hMem);

    if (azaza) {
        auto c = *azaza;
        GlobalUnlock(hMem);
        CloseClipboard();
        return c == '\0';
    }
    return true;
}
