//
// Created by alex2 on 26.11.2020.
//

#include "AClipboard.h"

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
    size_t length = 0;
    for (; *azaza && length < 50'000; ++length) {

    }
    if (length >= 50'000) {
        return {};
    }
    AString s = azaza;
    GlobalUnlock(hMem);
    CloseClipboard();
    return s;
}
#else
void AClipboard::copyToClipboard(const AString& text) {

}

AString AClipboard::pasteFromClipboard() {
    return "unsupported";
}
#endif