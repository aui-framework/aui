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


#include "IEventLoop.h"
#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * @brief Default event loop implementation.
 * @ingroup core
 */
class API_AUI_CORE AEventLoop: public IEventLoop {
public:
    ~AEventLoop() override;

    void notifyProcessMessages() override;

    /**
     * @brief Default loop.
     * @details
     * Until mRunning = true, calls iteraton() infinitely. The loop can be broken via stop() method.
     */
    void loop() override;

    void stop() {
        mRunning = false;
        notifyProcessMessages();
    }

    /*
     * @brief One loop iteration.
     * @details
     * Useful to define custom loops.
     *
     * If no tasks for current thread available, blocks the thread execution.
     */
    void iteration();

private:
    AMutex mMutex;
    AConditionVariable mCV;
    bool mNotified = false;
    bool mRunning = false;
};
