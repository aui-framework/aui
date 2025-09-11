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
// Created by nelonn on 9/12/25.
//

#include "APathView.h"

#include <sys/stat.h>

#if AUI_PLATFORM_WIN
struct _stat64 APathView::stat() const {
    struct _stat64 s = {0};
    std::wstring pathU16 = aui::win32::toWchar(*this);
    _wstat64(pathU16.c_str(), &s);
    return s;
}
#else
struct stat APathView::stat() const {
    struct stat s = {};
    AString nt_str(*this);
    ::stat(nt_str.c_str(), &s);
    return s;
}
#endif
