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

#include <AUI/Util/ACleanup.h>
#include "AUI/Util/AScheduler.h"
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
        mTimer = scheduler().timer(mPeriod, [this, self = aui::ptr::weak_from_this(this)] {
            if (auto v = self.lock()) {
                // this is valid
                emit fired;
            }
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
        ACleanup::afterEntry([] {
            thread->interrupt();
            thread->join();
            thread = nullptr;
        });
        return t;
    }();
    return thread;
}

AScheduler& ATimer::scheduler() {
#if AUI_PLATFORM_EMSCRIPTEN
    static AScheduler scheduler;
    return scheduler;
#else
    static struct GlobalScheduler {
        AScheduler scheduler;

        // make sure scheduler thread is started.
        _<AThread> thread = ATimer::timerThread();

        GlobalScheduler() {
        }
        ~GlobalScheduler() {
            scheduler.stop();

            // wait for the thread before destructing the scheduler.
            thread->interrupt();
            thread->join();
        }
    } global;
    return global.scheduler;
#endif
}
