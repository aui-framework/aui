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
#include <AUI/Platform/macos/MessageBox.h>

namespace aui {

void PlatformMacOS::setClipboardText(const AString& text) {
}

AString PlatformMacOS::getClipboardText() {
    return {};
}

AMessageBox::ResultButton PlatformMacOS::messageBoxShow(
    AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
    AMessageBox::Button b) {
    using namespace AMessageBox;


}

} // namespace aui
