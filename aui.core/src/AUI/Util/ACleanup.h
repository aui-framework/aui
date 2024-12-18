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

#include <functional>
#include <AUI/Common/AObject.h>
#include <AUI/Common/AQueue.h>


/**
 * @brief Cleanup helper.
 * @ingroup core
 */
class API_AUI_CORE ACleanup {
    friend void afterEntryCleanup();
public:
    using Callback = std::function<void()>;

    /**
     * @param callback callback that called after AUI_ENTRY but before native entry point exit.
     */
    static void afterEntry(Callback callback) {
        std::unique_lock lock(inst().mSync);
        inst().mCallbacks << std::move(callback);
    }

private:
    static ACleanup& inst();

    void afterEntryPerform();

    AMutex mSync;
    AQueue<Callback> mCallbacks;
};


