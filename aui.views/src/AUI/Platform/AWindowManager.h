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

#pragma once
#include <AUI/Thread/IEventLoop.h>
#include "AUI/Common/ATimer.h"
#include "AUI/Platform/Pipe.h"
#include "AUI/Util/AWatchdog.h"
#include "IRenderingContext.h"

class AWindow;
typedef union _XEvent XEvent;


class API_AUI_VIEWS AWindowManager: public IEventLoop {
    friend class AWindow;
    friend class AClipboard;
private:
    AWatchdog mWatchdog;
    _<ATimer> mHangTimer;

protected:
    IEventLoop::Handle mHandle;
    ADeque<_<AWindow>> mWindows;
    bool mLoopRunning = false;

#if AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX
    Pipe mNotifyPipe;
    std::atomic_bool mFastPathNotify = false;
    std::string mXClipboardText;

    void xProcessEvent(XEvent& ev);
    void xClipboardCopyImpl(const AString& text);
    AString xClipboardPasteImpl();
#endif

public:
    AWindowManager();
    ~AWindowManager() override;

    AWatchdog& watchdog() noexcept {
        return mWatchdog;
    }

    void removeAllWindows() {
        mWindows.clear();
    }

    void notifyProcessMessages() override;
    void loop() override;

    const ADeque<_<AWindow>>& getWindows() const {
        return mWindows;
    }

    void start() {
        mLoopRunning = true;
    }
    void stop() {
        mLoopRunning = false;
        notifyProcessMessages();
    }

    virtual void initNativeWindow(const IRenderingContext::Init& init);

    template<typename T>
    [[nodiscard]] ADeque<_<T>> getWindowsOfType() const {
        ADeque<_<T>> result;
        for (auto& w : mWindows) {
            if (auto c = _cast<T>(w)) {
                result << c;
            }
        }

        return std::move(result);
    }
};
