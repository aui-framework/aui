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

#import <UIKit/UIKit.h>
#include "AUI/Platform/AClipboard.h"

void AClipboard::copyToClipboard(const AString &text) {
    @autoreleasepool {
        [UIPasteboard generalPasteboard].string = [NSString stringWithUTF8String:text.c_str()];
    }
}

bool AClipboard::isEmpty() {
    return false;
}

AString AClipboard::pasteFromClipboard() {
    return AString();
}
