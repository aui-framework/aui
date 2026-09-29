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

#pragma once
#include <AUI/Thread/IEventLoop.h>
#include "AUI/Common/ATimer.h"
#include "AUI/Platform/Pipe.h"
#include "AUI/Util/AWatchdog.h"
#include "IRenderingContext.h"

class AWindow;

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

public:
    AWindowManager();
    ~AWindowManager() override;

    AWatchdog& watchdog() noexcept {
        return mWatchdog;
    }

    bool isLoopRunning() const { return mLoopRunning; }

    /**
     * @brief Whether the main loop should continue.
     * @details
     * The loop continues while it's not stopped and the application is held (see AApplication):
     * - by an explicit AApplication::hold(), or
     * - by at least one registered window if AApplication::quitOnLastWindowClosed is true, or
     * - unconditionally (until AApplication::quit()) if AApplication::quitOnLastWindowClosed is false.
     */
    [[nodiscard]]
    bool shouldKeepRunning() const;

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
