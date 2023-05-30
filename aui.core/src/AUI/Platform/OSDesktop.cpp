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

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Logging/ALogger.h>

#endif



class UIThread: public AAbstractThread {
public:
    UIThread() noexcept: AAbstractThread(std::this_thread::get_id()) {}

protected:
    void processMessagesImpl() override {
        assert(("AAbstractThread::processMessages() should not be called from other thread",
                mId == std::this_thread::get_id()));
        std::unique_lock lock(mQueueLock, std::defer_lock);

        using namespace std::chrono;
        using namespace std::chrono_literals;

        for (std::size_t i = 0; i < 10 && !mMessageQueue.empty() && lock.try_lock(); ++i)
        {
            auto f = std::move(mMessageQueue.front());
            mMessageQueue.pop_front();
            lock.unlock();
            auto time = util::measureExecutionTime<microseconds>(f.proc);

            if (time >= 1ms) {
                ALogger::warn("Performance")
                    << "Execution of a task took " << time.count() << "us to execute which may cause UI lag.\n"
                    << f.stacktrace
                    << " - ...\n";
            }
        }

        {
            static std::size_t prevRecord = 1;
            auto currentSize = mMessageQueue.size();
            if (auto r = currentSize / 10000; r > prevRecord) {
                prevRecord = r;
                ALogger::warn("Performance") << currentSize << " tasks for UI thread?";
            }
            if (currentSize > 1'000'000) {
                throw AException("{} tasks on UI thread - assuming application has frozen"_format(currentSize));
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

AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(AStringVector)) {
    AStringVector args;

    setupUIThread();
    ATimer::scheduler();

    AThread::setName("UI thread");

#if AUI_PLATFORM_WIN
    if (argc == 0) {
        // remove quotation marks
        AString argsRaw = GetCommandLineW();
        bool wrappedWithQuots = false;
        AString currentArg;
        for (auto& c : argsRaw) {
            switch (c) {
                case '\"':
                    wrappedWithQuots = !wrappedWithQuots;
                    break;

                case ' ':
                    if (!wrappedWithQuots) {
                        args << std::move(currentArg);
                        currentArg = {};
                        assert(currentArg.empty());
                        break;
                    }
                default:
                    currentArg += c;
            }
        }
        if (!currentArg.empty()) {
            args << std::move(currentArg);
        }
    }
#endif
    for (int i = 0; i < argc; ++i) {
        args << argv[i];
    }
    int r = -1;
    try {
        r = aui_entry(std::move(args));
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