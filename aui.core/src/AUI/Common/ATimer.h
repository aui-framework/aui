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

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Util/AScheduler.h>

/**
 * @brief Async timer.
 * @ingroup core
 * @details
 * Emits `fired` periodically with the specified period until stopped. The timer does not start on its own; call
 * `start()`. The first emission happens after one period elapses.
 *
 * ```cpp
 * class Clock: public AObject {
 * public:
 *     Clock() {
 *         connect(mTimer->fired, me::tick);
 *         mTimer->start();
 *     }
 *
 *     void tick() {
 *         ALogger::info("Clock") << "tick";
 *     }
 *
 * private:
 *     _<ATimer> mTimer = _new<ATimer>(1s);
 * };
 * ```
 *
 * `fired` is emitted from a dedicated timer thread. When the receiving object has
 * `AObject::setSlotsCallsOnlyOnMyThread` enabled (views do by default), the slot runs on the receiver's thread
 * instead.
 *
 * `stop()` and destroying the timer cancel any pending emission. `restart()` stops the timer and starts it again, so the
 * next emission occurs after a full period.
 *
 * See [core-async] for more on background work.
 */
class API_AUI_CORE ATimer : public AObject {
private:
    std::chrono::milliseconds mPeriod;

    AOptional<AScheduler::TimerHandle> mTimer;

    static _<AThread>& timerThread();

public:
    explicit ATimer(std::chrono::milliseconds period);
    ~ATimer() override;

    void restart();

    void start();
    void stop();
    bool isStarted();
    void setRunning(bool isStart) {
        if (isStart) {
            start();
        } else {
            stop();
        }
    }

    static AScheduler& scheduler();

signals:
    emits<> fired;
};
