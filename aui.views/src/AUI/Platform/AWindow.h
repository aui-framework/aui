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

#pragma once

#include "AUI/Views.h"
#include "AUI/Common/AString.h"

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"

#include "AUI/Platform/ABaseWindow.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Util/AMetric.h"
#include "AWindowNativePtr.h"
#include <AUI/Enum/WindowStyle.h>

#if AUI_PLATFORM_WIN
#include <windows.h>

#elif AUI_PLATFORM_ANDROID
#include <jni.h>
#elif AUI_PLATFORM_APPLE
#else

#endif

class ARender;
class AWindowManager;

class API_AUI_VIEWS AWindow: public ABaseWindow
{
    friend class OpenGLRenderingContext;
    friend class CommonRenderingContext;
    friend class SoftwareRenderingContext;
    friend class AWindowManager;
    friend struct painter;
public:
    AWindow(const AString& name = "My window", int width = 854_dp, int height = 500_dp, AWindow* parent = nullptr, WindowStyle ws = WindowStyle::DEFAULT) {
        windowNativePreInit(name, width, height, parent, ws);
        ARender::setWindow(this);
    }
    virtual ~AWindow();

    [[nodiscard]]
    unsigned frameMillis() const noexcept override {
        return mFrameMillis;
    }

    void redraw();

    void blockUserInput(bool blockUserInput = true) override;

    /**
     * @brief Checks whether last monitor frame is displayed and redraw will be efficient.
     *        If some object often updates UI thread for displaying some data it may cause extra CPU and GPU overload.
     *        AUI throttles window redraws and FPS does not go above 60 FPS but UI views may also cause extra CPU and
     *        GPU overload that does not have visual difference.
     * @return true if 16 milliseconds elapsed since last frame
     */
    static bool isRedrawWillBeEfficient();

    void setIcon(const AImage& image);

    /**
     * @brief Removes window from AWindowManager.
     */
    void quit();

    void setWindowStyle(WindowStyle ws);

    /**
     * @brief Minimizes window (hide window to the taskbar, iconifies)
     */
    void minimize();

    /**
     * @return true if window minimized (hidden in taskbar, iconified)
     */
    bool isMinimized() const;

    /**
     * @brief Maximizes window (makes window fullscreen)
     */
    void maximize();

    /**
     * @return true if window maximized (fullscreen)
     */
    bool isMaximized() const;

    /**
     * @brief Restores window (shows window from taskbar)
     */
    void restore();

    void flagRedraw() override;

    /**
     * Shows the window.
     * <dl>
     *   <dt><b>Behavior under UI tests</b></dt>
     *   <dd>Does not actually shows the window and don't even needed in graphics environment.</dd>
     * </dl>
     */
    void show();
    void close();
    void hide();

    [[nodiscard]]
    bool isClosed() const noexcept;

#if AUI_PLATFORM_WIN
    HWND getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_ANDROID
    jobject getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_APPLE
#else
    AWindowNativePtr getNativeHandle() { return mHandle; }
#endif

    auto nativeHandle() const {
        return mHandle;
    }

    const AString& getWindowTitle() const
    {
        return mWindowTitle;
    }

    glm::ivec2 getWindowPosition() const;

    void setPosition(glm::ivec2 position) override;
    void setSize(glm::ivec2 size) override;
    void setGeometry(int x, int y, int width, int height) override;


    void onFocusAcquired() override;
    void onFocusLost() override;

    void onKeyDown(AInput::Key key) override;
    void onKeyRepeat(AInput::Key key) override;

    void setFocusNextViewOnTab(bool value) {
        mFocusNextViewOnTab = value;
    }

    /**
     * Wraps your AView to window.
     * @param title window title
     * @param view view to wrap
     * @return created window, AWindow::show() is not called
     */
    static _<AWindow> wrapViewToWindow(const _<AView>& view, const AString& title, int width = 854_dp, int height = 500_dp, AWindow* parent = nullptr, WindowStyle ws = WindowStyle::DEFAULT);

    /**
     * @return Current window for current thread.
     */
    static ABaseWindow* current();

    /**
     * @brief Translates coordinates from the coordinate space of this window to the coordinate space of another window.
     * @param position coordinates in the space of this window
     * @param other other window
     * @return coordinates in the space of the other window
     */
    [[nodiscard]] glm::ivec2 mapPositionTo(const glm::ivec2& position, _<AWindow> other);

    /**
     * @brief Translates coordinates from the coordinate space of this window to the coordinate space of the monitor.
     * @param position coordinates in the space of this window
     * @return the coordinates in space of the monitor
     */
    [[nodiscard]] glm::ivec2 unmapPosition(const glm::ivec2& position);

    /**
     * @brief Translates coordinates from the monitor's coordinate space to the coordinate space of this window.
     * @param position the coordinate in screen space
     * @return coordinates in the space of this window
     */
    [[nodiscard]] glm::ivec2 mapPosition(const glm::ivec2& position);

    _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) override;

    void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) override;
    virtual void onCloseButtonClicked();

    void forceUpdateCursor() override;

    void showTouchscreenKeyboardImpl() override;
    void hideTouchscreenKeyboardImpl() override;

    /**
     * @brief Moves the window to the center of monitor.
     * @details
     * When using in series with setSize(), do the setSize() first, when moveToCenter().
     */
    void moveToCenter();

signals:
    emits<> closed;
    emits<> redrawn;
    emits<> shown;

    /**
     * @brief Window is moving.
     * @param client area position.
     */
    emits<glm::vec2> moving;

    /**
     * @brief Window is maximized.
     */
    emits<> maximized;

    /**
     * @brief Window is minimized (hidden to the taskbar, iconified).
     */
    emits<> minimized;

    /**
     * @brief Window is restored (shown from the taskbar, deiconified).
     */
    emits<> restored;

    bool consumesClick(const glm::ivec2& pos) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;

    void flagUpdateLayout() override;
protected:
#if AUI_PLATFORM_WIN
    HICON mIcon = nullptr;
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
#endif
    AWindowNativePtr mHandle = 0; // on linux AWindowNativePtr is not a pointer type so using zero here
    WindowStyle mWindowStyle = WindowStyle::DEFAULT;

    virtual void doDrawWindow();
    virtual void onClosed();

    void windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws);

    /**
     * @brief Enables drag-n-drop for this window.
     */
    void allowDragNDrop();

    /**
     * @brief Constructor for custom initialization logic
     * @note Please call windowNativePreInit
     */
    AWindow(std::nullptr_t) {}

    void createDevtoolsWindow() override;
    float fetchDpiFromSystem() const override;

    /**
     * @brief defines if the next view must be focused on tab button pressed
     */
    bool mFocusNextViewOnTab = false;

private:
#if AUI_PLATFORM_WIN

#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX
    /**
     * _NET_WM_SYNC_REQUEST (resize flicker fix) update request counter
     */
    struct {
        uint32_t lo = 0;
        uint32_t hi = 0;
        /* XID */ unsigned long counter;
    } mXsyncRequestCounter;
    bool mWasMaximized = false;

    void* mIC = nullptr; // input context
#endif
#ifdef AUI_PLATFORM_MACOS
    bool mRedrawFlag = false;
#else
    bool mRedrawFlag = true;
#endif
    bool mUpdateLayoutFlag = true;
    AString mWindowClass;
    AWindow* mParentWindow;

    unsigned mFrameMillis = 1;

    /**
     * @brief Handles self shared pointer.
     */
    _<AWindow> mSelfHolder;

    AString mWindowTitle;

#if AUI_PLATFORM_WIN
    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
#else
    unsigned long xGetWindowProperty(unsigned long property, unsigned long type, unsigned char** value) const;
    void xSendEventToWM(unsigned long atom, long a, long b, long c, long d, long e) const;
#endif

};
