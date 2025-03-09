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
#include "AObject.h"
#include "ASignal.h"
#include "AUI/Util/AScheduler.h"

/**
 * @brief Async timer.
 * @ingroup core
 */
class API_AUI_CORE ATimer : public AObject {
private:
    std::chrono::milliseconds mPeriod;

    AOptional<AScheduler::TimerHandle> mTimer;

    static _<AThread>& timerThread();

public:
    explicit ATimer(std::chrono::milliseconds period);
    ~ATimer();

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
