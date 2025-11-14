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
// Created by nelonn on 11/14/25.
//

#include "Platform.h"

#include <Windows.h>
#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/AWindow.h>

void PlatformWin32::setClipboardText(const AString& text) {
    auto wString = aui::win32::toWchar(text);
    const size_t len = (text.length() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), wString.data(), len);
    GlobalUnlock(hMem);
    OpenClipboard(nullptr);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
}

AString PlatformWin32::getClipboardText() {
    OpenClipboard(nullptr);
    HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
    std::wstring_view memView = static_cast<const wchar_t*>(GlobalLock(hMem));
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

AMessageBox::ResultButton PlatformWin32::messageBoxShow(
    AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
    AMessageBox::Button b) {
    using namespace AMessageBox;

    HWND window = parent ? parent->getNativeHandle() : nullptr;

    long flags = 0;

    // Icons
    switch (icon)
    {
        case Icon::INFO:
            flags |= MB_ICONINFORMATION;
            break;
        case Icon::WARNING:
            flags |= MB_ICONWARNING;
            break;
        case Icon::CRITICAL:
            flags |= MB_ICONSTOP;
            break;
        default: break;
    }


    // Buttons
    switch (b) {
        case Button::OK:
            flags |= MB_OK;
            break;

        case Button::OK_CANCEL:
            flags |= MB_OKCANCEL;
            break;

        case Button::YES_NO:
            flags |= MB_YESNO;
            break;

        case Button::YES_NO_CANCEL:
            flags |= MB_YESNOCANCEL;
            break;
    }

    auto u16message = aui::win32::toWchar(message);
    auto u16title = aui::win32::toWchar(title);
    switch (::MessageBox(window, u16message.c_str(), u16title.c_str(), flags)) {
        case IDOK:
            return ResultButton::OK;
        case IDCANCEL:
            return ResultButton::CANCEL;
        case IDYES:
            return ResultButton::YES;
        case IDNO:
            return ResultButton::NO;
        default: break;
    }
    return ResultButton::INVALID;
}
