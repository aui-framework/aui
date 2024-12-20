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

#pragma once

#import <Cocoa/Cocoa.h>

class AWindow;

/**
 * @brief AUI viewport representation for Cocoa.
 * @details
 * This is the view that lives in the Cocoa view hierarchy.
 */
@interface AUIView : NSView<NSTextInputClient>
    - (AUIView*) initWithWindow:(AWindow*)window;
@end