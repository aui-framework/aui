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

#include <chrono>
#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Common/AException.h>
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/Util.h"
#include "AUI/Util/kAUI.h"
#include <AUI/Util/ACleanup.h>
#include <AUI/Common/ATimer.h>
#include <AUI/Platform/Entry.h>

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Logging/ALogger.h>

#endif


using namespace std::chrono_literals;
static constexpr auto MAX_PROCESSING_ITERATIONS_PER_FRAME = 100'000;
static constexpr auto MAX_PROCESSING_TIME_FOR_ALL_TASKS = 10ms;
static constexpr auto MAX_PROCESSING_TIME_FOR_ONE_TASK = 1ms;

class UIThread: public AAbstractThread {
public:
    UIThread() noexcept: AAbstractThread(std::this_thread::get_id()) {}

protected:
    void processMessagesImpl() override {
        AUI_ASSERTX(mId == std::this_thread::get_id(),
                    "AAbstractThread::processMessages() should not be called from other thread");
        std::unique_lock lock(mMessageQueue.sync(), std::defer_lock);

        using namespace std::chrono;

        auto beginTime = system_clock::now();
        for (std::size_t i = 0; i <= MAX_PROCESSING_ITERATIONS_PER_FRAME && !mMessageQueue.messages().empty() && lock.try_lock(); ++i)
        {
            auto f = std::move(mMessageQueue.messages().front());
            mMessageQueue.messages().pop_front();
            lock.unlock();
            auto time = util::measureExecutionTime<microseconds>(f);
            // TODO dynamically enable/disable logging
            /*
            ALOG_DEBUG("Performance")
                    << "Executing:\n"
                    << f.stacktrace
                    << " - ...\n";
            */

            if (time >= MAX_PROCESSING_TIME_FOR_ONE_TASK) {
                ALogger::warn("Performance")
                    << "Execution of a task took " << time.count() << "us to execute which may cause UI lag.\n"
                    << " - ...\n";
            }

            if (i % 10000 == 0) {
                if (system_clock::now() - beginTime >= MAX_PROCESSING_TIME_FOR_ALL_TASKS) {
                    break;
                }
            }
        }
        lock.lock();
        {
            static std::size_t prevRecord = 1;
            auto currentSize = mMessageQueue.messages().size();
            if (auto r = currentSize / 10000; r > prevRecord) {
                prevRecord = r;
                ALogger::warn("Performance") << currentSize << " tasks for UI thread?";
            }
            if (currentSize > 1'000'000) {
                // throw AException("{} tasks on UI thread - assuming application has frozen"_format(currentSize));
            }
        }
    }
};

void setupUIThread() noexcept {
    AAbstractThread::threadStorage() = _new<UIThread>();
}

void afterEntryCleanup() {
    ACleanup::inst().afterEntryPerform();
}

static ACommandLineArgs& argsImpl() {
    static ACommandLineArgs args;
    return args;
}

const ACommandLineArgs& aui::args() noexcept {
    return argsImpl();
}

namespace aui::detail {
    int argc;
    char** argv;
}

AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    aui::detail::argc = argc;
    aui::detail::argv = argv;
    setupUIThread();
    ATimer::scheduler();
    {
        [[maybe_unused]] auto mainThread = AThread::main();
    }

    AThread::setName("UI thread");

#if AUI_PLATFORM_WIN
    if (argc == 0) {
        // remove quotation marks
        AString argsRaw(reinterpret_cast<const char16_t*>(GetCommandLineW()));
        bool wrappedWithQuots = false;
        AString currentArg;
        for (auto c : argsRaw) {
            switch (c) {
                case '\"':
                    wrappedWithQuots = !wrappedWithQuots;
                    break;

                case ' ':
                    if (!wrappedWithQuots) {
                        argsImpl() << std::move(currentArg);
                        currentArg = {};
                        AUI_ASSERT(currentArg.empty());
                        break;
                    }
                default:
                    currentArg += c;
            }
        }
        if (!currentArg.empty()) {
            argsImpl() << std::move(currentArg);
        }
    }
#endif
    for (int i = 0; i < argc; ++i) {
        argsImpl() << argv[i];
    }
    int r = -1;

#ifdef AUI_CATCH_UNHANDLED
    extern void aui_init_signal_handler();
    aui_init_signal_handler();
#endif
    try {
        r = aui_entry(argsImpl());
        if (auto el = AThread::current()->getCurrentEventLoop()) {
            el->loop();
        }
    } catch (const AException& e) {
        ALogger::err("AUI") << "Uncaught exception: " << e;
    } catch (const std::exception& e) {
        ALogger::err("AUI") << "Uncaught std exception: " << e.what();
    } catch (...) {
        ALogger::err("AUI") << "Uncaught unknown exception";
    }
    afterEntryCleanup();
    return r;
}
#endif
