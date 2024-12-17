/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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


bool AClipboard::isEmpty() {
    return pasteFromClipboard().empty();
}
void AClipboard::copyToClipboard(const AString& text) {
    AWindow::current()->getWindowManager().xClipboardCopyImpl(text);
}
AString AClipboard::pasteFromClipboard() {
    return AWindow::current()->getWindowManager().xClipboardPasteImpl();
}