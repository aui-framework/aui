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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include "AUI/Util/AScheduler.h"
#include "AUI/Traits/iterators.h"


using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    class TimeLog: public AVector<high_resolution_clock::time_point> {
    public:
        TimeLog(): mBegin(high_resolution_clock::now()) {

        }

        void push() {
            push_back(high_resolution_clock::now());
        }

        void compare(const AVector<std::chrono::microseconds>& values) {
            ASSERT_EQ(size(), values.size());

            for (const auto&[v1, v2] : aui::zip(*this, values)) {
                EXPECT_NEAR(duration_cast<microseconds>(v1 - mBegin).count(), v2.count(), 100'000);
            }
        }

        high_resolution_clock::time_point mBegin;
    };
}


TEST(Scheduler, Enqueue) {
    TimeLog log;
    AScheduler scheduler;


    scheduler.enqueue(500ms, [&] {
        log.push();
    });

    scheduler.enqueue(600ms, [&] {
        log.push();
    });

    while (!scheduler.emptyTasks()) {
        scheduler.iteration();
    }

    log.compare({
        500ms,
        600ms,
    });
}


TEST(Scheduler, Timer) {
    TimeLog log;
    AScheduler scheduler;


    scheduler.timer(100ms, [&] {
        log.push();
    });

    for (int i = 0; i < 20; ++i) {
        scheduler.iteration();
    }

    log.compare({
        100ms,
        200ms,
        300ms,
        400ms,
        500ms,
        600ms,
        700ms,
        800ms,
        900ms,
        1000ms,
        1100ms,
        1200ms,
        1300ms,
        1400ms,
        1500ms,
        1600ms,
        1700ms,
        1800ms,
        1900ms,
    });
}

TEST(Scheduler, TimerCancellation) {
    TimeLog log;
    AScheduler scheduler;


    auto t = scheduler.timer(100ms, [&] {
        log.push();
    });

    for (int i = 0; i < 20; ++i) {
        scheduler.iteration(ASchedulerIteration::DONT_BLOCK_INFINITELY);
        if (i == 10) {
            scheduler.removeTimer(t);
        }
    }

    log.compare({
        100ms,
        200ms,
        300ms,
        400ms,
        500ms,
        600ms,
        700ms,
        800ms,
        900ms,
        1000ms,
    });
}
