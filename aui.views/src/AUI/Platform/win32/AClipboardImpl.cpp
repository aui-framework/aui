/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 26.11.2020.
//

#include "AUI/Platform/AClipboard.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/AWindowManager.h"

#include <Windows.h>

void AClipboardImpl::copyToClipboard(const AString& text) {
    const size_t len = text.length() * 2 + 2;
    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), text.data(), len);
    GlobalUnlock(hMem);
    OpenClipboard(nullptr);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
}

AString AClipboardImpl::pasteFromClipboard() {
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

bool AClipboardImpl::isEmpty() {
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
