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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include "AUI/Thread/AThread.h"
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

            // a task is never early, but how late it is depends on the system: the wait of a thread may take hundreds of
            // milliseconds longer than requested on a virtual machine, so there is no upper bound
            for (const auto&[v1, v2] : aui::zip(*this, values)) {
                EXPECT_GE(duration_cast<microseconds>(v1 - mBegin).count(), v2.count());
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

    // counts firings, not iterations: an iteration may end with no firing, as the wait may finish a bit before the time
    // of the timer
    while (log.size() < 19) {
        AThread::sleep(10ms); // minimize unsync
        scheduler.iteration();
    }
    log.resize(19); // a late iteration fires what it has missed at once

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

    // counts firings, not iterations; see Scheduler.Timer
    while (log.size() < 3) {
        scheduler.iteration(ASchedulerIteration::DONT_BLOCK_INFINITELY);
    }
    scheduler.removeTimer(t);
    const auto fired = log.size();

    AThread::sleep(300ms);
    scheduler.iteration(ASchedulerIteration::DONT_BLOCK);
    EXPECT_EQ(log.size(), fired);
}
