// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/Util/ACleanup.h>
#include "ATimer.h"

ATimer::ATimer(std::chrono::milliseconds period):
	mPeriod(period)
{
    assert(("zero period?", period.count() != 0));
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
