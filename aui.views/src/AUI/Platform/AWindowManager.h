/**
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

#pragma once
#include <AUI/Thread/IEventLoop.h>
#include "IRenderingContext.h"

class AWindow;
typedef union _XEvent XEvent;


class API_AUI_VIEWS AWindowManager: public IEventLoop {
    friend class AWindow;
    friend class AClipboard;
private:
    IEventLoop::Handle mHandle;
    ADeque<_<AWindow>> mWindows;
    bool mLoopRunning = false;

#if AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX
    AMutex mXNotifyLock;
    AConditionVariable mXNotifyCV;
    std::string mXClipboardText;

    void xProcessEvent(XEvent& ev);
    void xClipboardCopyImpl(const AString& text);
    AString xClipboardPasteImpl();
#endif

public:
    AWindowManager();
    ~AWindowManager() override;

    void notifyProcessMessages() override;
    void loop() override;

    const ADeque<_<AWindow>>& getWindows() const {
        return mWindows;
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
