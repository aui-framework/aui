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

#if !(AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS)
#include <AUI/api.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Common/AException.h>
#include "AUI/Logging/ALogger.h"
#include "AUI/Util/Util.h"

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
        std::unique_lock lock(mQueueLock);

        using namespace std::chrono;
        using namespace std::chrono_literals;

        while (!mMessageQueue.empty())
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

            lock.lock();
        }
    }
};

void setupUIThread() noexcept {
    AAbstractThread::threadStorage() = _new<UIThread>();
}

AUI_EXPORT int aui_main(int argc, char** argv, int(*aui_entry)(const AStringVector&)) {
    AStringVector args;

    setupUIThread();

    // renames all threads to "UI thread" on linux
#if !AUI_PLATFORM_LINUX
    AThread::setName("UI thread");
#endif

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
        r = aui_entry(args);
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
    return r;
}
#endif