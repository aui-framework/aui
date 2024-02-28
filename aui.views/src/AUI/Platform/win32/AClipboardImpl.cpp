// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
    auto azaza = (const wchar_t*)GlobalLock(hMem);

	if (azaza) {
        size_t length = 0;
        for (; azaza[length] && length < 50'000; ++length) {

        }
        if (length >= 50'000) {
            GlobalUnlock(hMem);
            CloseClipboard();
            return {};
        }
        AString s = azaza;
        GlobalUnlock(hMem);
        CloseClipboard();
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
