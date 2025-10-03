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

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>
#import <UIKit/UIKit.h>



float APlatform::getDpiRatio()
{
    return float([[UIScreen mainScreen] scale]);
}

void APlatform::openUrl(const AUrl &url) {
    @autoreleasepool {
        UIApplication *application = [UIApplication sharedApplication];
        NSURL *URL = [NSURL URLWithString:[NSString stringWithUTF8String: url.full().c_str()]];
        [application openURL:URL options:@{} completionHandler:nil];
    }
}
