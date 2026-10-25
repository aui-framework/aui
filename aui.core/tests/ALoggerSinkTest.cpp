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

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include "AUI/Common/AVector.h"
#include "AUI/Logging/ALogger.h"

namespace {
/// Removes the sink on destruction so a failing test doesn't leak it into the others.
struct ScopedSink {
    _<ALogger::ISink> sink;
    explicit ScopedSink(_<ALogger::ISink> sink) : sink(std::move(sink)) { ALogger::global().addSink(this->sink); }
    ~ScopedSink() { ALogger::global().removeSink(sink); }
};
}   // namespace

TEST(ALoggerSink, SeveralSinksReceiveEntries) {
    AVector<AString> first, second;
    ScopedSink a(_new<ALogger::CallbackSink>([&](const ALogger::Entry& e) {
        first << "{}:{}"_format(e.prefix, e.message);
    }));
    ScopedSink b(_new<ALogger::CallbackSink>([&](const ALogger::Entry& e) {
        second << "{}:{}"_format(e.prefix, e.message);
    }));
    ALogger::info("SinkTest") << "hello";
    EXPECT_EQ(first, AVector<AString>{ "SinkTest:hello" });
    EXPECT_EQ(second, AVector<AString>{ "SinkTest:hello" });
}

TEST(ALoggerSink, RemovedSinkStopsReceiving) {
    int calls = 0;
    auto sink = _new<ALogger::CallbackSink>([&](const ALogger::Entry&) { ++calls; });
    ALogger::global().addSink(sink);
    ALogger::info("SinkTest") << "one";
    EXPECT_EQ(calls, 1);
    ALogger::global().removeSink(sink);
    ALogger::global().removeSink(sink);   // harmless
    ALogger::info("SinkTest") << "two";
    EXPECT_EQ(calls, 1);
}

TEST(ALoggerSink, LevelIsPassed) {
    ALogger::Level seen = ALogger::INFO;
    ScopedSink s(_new<ALogger::CallbackSink>([&](const ALogger::Entry& e) { seen = e.level; }));
    ALogger::warn("SinkTest") << "careful";
    EXPECT_EQ(seen, ALogger::WARN);
}

TEST(ALoggerSink, CustomSinkAndFlush) {
    struct MySink : ALogger::ISink {
        int logged = 0;
        int flushed = 0;
        void log(const ALogger::Entry&) override { ++logged; }
        void flush() override { ++flushed; }
    };
    auto sink = _new<MySink>();
    ScopedSink s(sink);
    ALogger::info("SinkTest") << "x";
    ALogger::global().flush();
    EXPECT_EQ(sink->logged, 1);
    EXPECT_EQ(sink->flushed, 1);
}

TEST(ALoggerSink, LegacyOnLoggedCoexistsWithSinks) {
    int legacy = 0, sinkCalls = 0;
    ALogger::global().onLogged([&](const AString&, const AString&, ALogger::Level) { ++legacy; });
    ScopedSink s(_new<ALogger::CallbackSink>([&](const ALogger::Entry&) { ++sinkCalls; }));
    ALogger::info("SinkTest") << "x";
    EXPECT_EQ(legacy, 1);
    EXPECT_EQ(sinkCalls, 1);

    // replaces only the legacy one
    int legacy2 = 0;
    ALogger::global().onLogged([&](const AString&, const AString&, ALogger::Level) { ++legacy2; });
    ALogger::info("SinkTest") << "y";
    EXPECT_EQ(legacy, 1);
    EXPECT_EQ(legacy2, 1);
    EXPECT_EQ(sinkCalls, 2);

    ALogger::global().onLogged(nullptr);
    ALogger::info("SinkTest") << "z";
    EXPECT_EQ(legacy2, 1);
    EXPECT_EQ(sinkCalls, 3);
}

TEST(ALoggerSink, SinkMayRemoveItself) {
    _<ALogger::ISink> sink;
    int calls = 0;
    sink = _new<ALogger::CallbackSink>([&](const ALogger::Entry&) {
        ++calls;
        ALogger::global().removeSink(sink);
    });
    ALogger::global().addSink(sink);
    ALogger::info("SinkTest") << "once";
    ALogger::info("SinkTest") << "twice";
    EXPECT_EQ(calls, 1);
}

TEST(ALoggerSink, LoggingFromSeveralThreads) {
    std::atomic<int> calls = 0;
    ScopedSink s(_new<ALogger::CallbackSink>([&](const ALogger::Entry&) { ++calls; }));
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([] {
            for (int j = 0; j < 50; ++j) {
                ALogger::info("SinkTest") << "message";
            }
        });
    }
    for (auto& t : threads) t.join();
    EXPECT_EQ(calls, 200);
}
