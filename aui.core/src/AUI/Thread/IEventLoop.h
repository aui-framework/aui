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

#include <AUI/Core.h>

class API_AUI_CORE IEventLoop {
public:
    IEventLoop() {}
    virtual ~IEventLoop() {}

    /**
     * @brief Notifies this IEventLoop that its thread got a new message to process.
     */
    virtual void notifyProcessMessages() = 0;

    /**
     * @brief Do message processing loop.
     */
    virtual void loop() = 0;

    class API_AUI_CORE Handle {
    private:
        IEventLoop* mPrevEventLoop;
        IEventLoop* mCurrentEventLoop;

    public:
        explicit Handle(IEventLoop* loop);
        ~Handle();
    };
};
