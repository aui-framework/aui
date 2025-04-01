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

#pragma once

#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * @brief One-shot event communication.
 * @ingroup core
 * @details
 * Used for guaranteed signal from slave thread to master thread (i.e. wait till slave thread starts).
 */
class [[deprecated("Use AFuture<void> instead")]] ACutoffSignal {
public:
    ACutoffSignal() = default;

    /**
     * @brief Flags a signal, causing the waitForSignal() to never block.
     */
    void makeSignal() {
        std::unique_lock lock(mMutex);
        mSignaled = true;
        mCV.notify_all();
    }

    /**
     * @brief Resets the cut off state so the waitForSignal() function would block again.
     */
     void resetSignal() noexcept {
        mSignaled = false;
    }

    /**
     * @brief Wait for makeSignal().
     * @details
     * waitForSignal() blocks the thread until makeSignal() is not called by other thread.
     *
     * Unlike condition variable, the second call to makeSignal() will not block. To override this behaviour, use the
     * resetSignal() function.
     */
    void waitForSignal() {
        if (!mSignaled) {
            std::unique_lock lock(mMutex);
            mCV.wait(lock, [this] {
                return mSignaled;
            });
        }
    }

private:
    AMutex mMutex;
    AConditionVariable mCV;
    bool mSignaled = false;
};


