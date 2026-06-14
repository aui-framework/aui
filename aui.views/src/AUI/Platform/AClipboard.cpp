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

#include "AClipboard.h"

#include <AUI/Platform/APlatform.h>

bool AClipboard::isEmpty() {
    return pasteFromClipboard().empty();
}

void AClipboard::copyToClipboard(const AString& text) {
    APlatform::current().setClipboardText(text);
}

AString AClipboard::pasteFromClipboard() {
    return APlatform::current().getClipboardText();
}
