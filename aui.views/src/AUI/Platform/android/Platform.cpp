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

#include <AUI/Platform/AMessageBox.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/android/OSAndroid.h>

namespace aui {

void PlatformAndroid::setClipboardText(const AString& text) {
    com::github::aui::android::Platform::copyToClipboard(text);
}

AString PlatformAndroid::getClipboardText() {
    return {};
}

AMessageBox::ResultButton PlatformAndroid::messageBoxShow(
    AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
    AMessageBox::Button b) {
    using namespace AMessageBox;
    com::github::aui::android::Platform::showMessageBox(title, message);
    return ResultButton::INVALID;
}

} // namespace aui
