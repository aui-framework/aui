/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

#if AUI_PLATFORM_LINUX
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
        auto windows = std::move(mWindows); // keeping it safe
        windows.clear();
    }

    void closeAllWindows();
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
