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

#include <AUI/Platform/AInput.h>
#include <AUI/Platform/linux/IPlatformAbstraction.h>

AInput::Key AInput::fromNative(int k) {
    auto* platform = IPlatformAbstraction::currentSafe();
    return platform ? platform->inputFromNative(k) : AInput::UNKNOWN;
}

int AInput::toNative(Key key) {
    auto* platform = IPlatformAbstraction::currentSafe();
    return platform ? platform->inputToNative(key) : 0;
}

bool AInput::native::isKeyDown(Key k) {
    auto* platform = IPlatformAbstraction::currentSafe();
    return platform ? platform->inputIsKeyDown(k) : false;
}
