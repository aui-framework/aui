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

#include "AClipboard.h"
#include "AWindow.h"
#include "AWindowManager.h"

#ifdef _WIN32
#include <windows.h>

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

#elif defined(__ANDROID__)

void AClipboard::copyToClipboard(const AString &text) {
    // stub
}

bool AClipboard::isEmpty() {
    // stub
    return false;
}

AString AClipboard::pasteFromClipboard() {
    // stub
    return AString();
}

#else

bool AClipboard::isEmpty() {
    return pasteFromClipboard().empty();
}
void AClipboard::copyToClipboard(const AString& text) {
    AWindow::current()->getWindowManager().xClipboardCopyImpl(text);
}
AString AClipboard::pasteFromClipboard() {
    return AWindow::current()->getWindowManager().xClipboardPasteImpl();
}
#endif