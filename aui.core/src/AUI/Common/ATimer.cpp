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

#include <AUI/Util/ACleanup.h>
#include "ATimer.h"

ATimer::ATimer(std::chrono::milliseconds period):
	mPeriod(period)
{
    AUI_ASSERTX(period.count() != 0, "zero period?");
}

ATimer::~ATimer()
{
	stop();
}

void ATimer::restart()
{
    stop();
    start();
}

void ATimer::start()
{
    if (!mTimer) {
        ATimer::scheduler();
        mTimer = scheduler().timer(mPeriod, [this] {
            emit fired;
        });
    }
}

void ATimer::stop()
{
    if (mTimer) {
        scheduler().removeTimer(*mTimer);
        mTimer = std::nullopt;
    }
}

bool ATimer::isStarted()
{
	return mTimer.hasValue();
}

_<AThread>& ATimer::timerThread() {
    static _<AThread> thread = [] {
        auto t = _new<AThread>([&]()
            {
                AThread::setName("Timer thread");
                IEventLoop::Handle h(&ATimer::scheduler());
                ATimer::scheduler().loop();
            });
        t->start();
#if !AUI_PLATFORM_WIN
        std::atexit([] {
            thread->interrupt();
        });
#endif
        return t;
    }();
    return thread;
}

AScheduler& ATimer::scheduler() {
    static AScheduler scheduler;
    ATimer::timerThread();
    return scheduler;
}
