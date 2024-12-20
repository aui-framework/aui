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
// Created by Alex2772 on 7/16/2022.
//

#include "ACleanup.h"

ACleanup& ACleanup::inst() {
    static ACleanup c;
    return c;
}

void ACleanup::afterEntryPerform() {
    std::unique_lock lock(mSync);

    while (!mCallbacks.empty()) {
        auto callback = std::move(mCallbacks.front());
        mCallbacks.pop();

        lock.unlock(); // avoid deadlock
        callback();
        lock.lock();
    }
}

