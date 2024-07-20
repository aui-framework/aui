﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AUI/Platform/AMessageBox.h"
#include <Windows.h>
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Platform/AWindow.h"

AMessageBox::ResultButton AMessageBox::show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b)
{
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

    switch (::MessageBox(window, aui::win32::toWchar(message), aui::win32::toWchar(title), flags)) {
        case IDOK:
            return ResultButton::OK;
        case IDCANCEL:
            return ResultButton::CANCEL;
        case IDYES:
            return ResultButton::YES;
        case IDNO:
            return ResultButton::NO;
    }
    return ResultButton::INVALID;
}
