/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
        ATimer::timerThread();
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
    ATimer::scheduler();
    static _<AThread> thread = [] {
        auto t = _new<AThread>([&]()
            {
                AThread::setName("Timer thread");
                IEventLoop::Handle h(&ATimer::scheduler());
                ATimer::scheduler().loop();
            });
        t->start();
        return t;
    }();
    std::atexit([] {
        thread->interrupt();
        thread->join();
    });
    return thread;
}

AScheduler& ATimer::scheduler() {
    static AScheduler scheduler;
    return scheduler;
}
