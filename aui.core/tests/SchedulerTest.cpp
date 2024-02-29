// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
