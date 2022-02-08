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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xatom.h>
#endif

class Render;
class AWindowManager;

class API_AUI_VIEWS AWindow: public ABaseWindow, public std::enable_shared_from_this<AWindow>
{
    friend class OpenGLRenderingContext;
    friend class CommonRenderingContext;
    friend class AWindowManager;
    friend struct painter;
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
        XID counter;
    } mXsyncRequestCounter;
    bool mWasMaximized = false;
#endif
#ifdef AUI_PLATFORM_MACOS
    bool mRedrawFlag = false;
#else
    bool mRedrawFlag = true;
#endif
    bool mUpdateLayoutFlag = true;
    AString mWindowClass;
    AWindow* mParentWindow;

    /**
     * \brief Handles self shared pointer.
     */
    _<AWindow> mSelfHolder;

    AString mWindowTitle;

#if AUI_PLATFORM_WIN
    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
#else
    unsigned long xGetWindowProperty(Atom property, Atom type, unsigned char** value) const;
    void xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const;
#endif

protected:
#if AUI_PLATFORM_WIN
	HICON mIcon = nullptr;
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
#else
    XIC mIC;
#endif
    AWindowNativePtr mHandle = 0; // on linux AWindowNativePtr is not a pointer type so using zero here
    WindowStyle mWindowStyle = WindowStyle::DEFAULT;

    virtual void doDrawWindow();
    virtual void onClosed();

    void windowNativePreInit(const AString& name, int width, int height, AWindow* parent, WindowStyle ws);

    /**
     * \brief Constructor for custom initialization logic
     * \note Please call windowNativePreInit
     */
    AWindow(std::nullptr_t) {}

    void createDevtoolsWindow() override;
    float fetchDpiFromSystem() const override;

public:
    AWindow(const AString& name = "My window", int width = 854_dp, int height = 500_dp, AWindow* parent = nullptr, WindowStyle ws = WindowStyle::DEFAULT) {
        windowNativePreInit(name, width, height, parent, ws);
        Render::setWindow(this);
    }
    virtual ~AWindow();

    void redraw();


    _<AView> determineSharedPointer() override;

    /**
     * \brief Checks whether last monitor frame is displayed and redraw will be efficient.
     *        If some object often updates UI thread for displaying some data it may cause extra CPU and GPU overload.
     *        AUI throttles window redraws and FPS does not go above 60 FPS but UI views may also cause extra CPU and
     *        GPU overload that does not have visual difference.
     * \return true if 16 milliseconds elapsed since last frame
     */
    static bool isRedrawWillBeEfficient();

    void setIcon(const AImage& image);

    /**
     * \brief Removes window from AWindowManager.
     */
    void quit();

    void setWindowStyle(WindowStyle ws);

    /**
     * \brief Minimizes window (hide window to the taskbar, iconifies)
     */
    void minimize();

    /**
     * \return true if window minimized (hidden in taskbar, iconified)
     */
    bool isMinimized() const;

    /**
     * \brief Maximizes window (makes window fullscreen)
     */
    void maximize();

    /**
     * \return true if window maximized (fullscreen)
     */
    bool isMaximized() const;

    /**
     * \brief Restores window (shows window from taskbar)
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

#if AUI_PLATFORM_WIN
    HWND getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_ANDROID
    jobject getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_APPLE
#else
    Window getNativeHandle() { return mHandle; }
#endif

    auto nativeHandle() const {
        return mHandle;
    }

    const AString& getWindowTitle() const
    {
        return mWindowTitle;
    }

    glm::ivec2 getWindowPosition() const;

    void setPosition(const glm::ivec2& position) override;
    void setSize(int width, int height) override;
    void setGeometry(int x, int y, int width, int height) override;


    void onFocusAcquired() override;
    void onFocusLost() override;

    void onKeyRepeat(AInput::Key key) override;

    /**
     * Wraps your AView to window.
     * @param title window title
     * @param view view to wrap
     * @return created window, AWindow::show() is not called
     */
    static _<AWindow> wrapViewToWindow(const _<AView>& view, const AString& title, int width = 854_dp, int height = 500_dp, AWindow* parent = nullptr, WindowStyle ws = WindowStyle::DEFAULT);

    /**
     * \return Current window for current thread.
     */
    static ABaseWindow* current();

    /**
     * \brief Determines whether views should display hover animations.
     * \return false when any keyboard button is pressed
     */
    static bool shouldDisplayHoverAnimations();

    /**
     * \brief Translates coordinates from the coordinate space of this window to the coordinate space of another window.
     * \param position coordinates in the space of this window
     * \param other other window
     * \return coordinates in the space of the other window
     */
    [[nodiscard]] glm::ivec2 mapPositionTo(const glm::ivec2& position, _<AWindow> other);

    /**
     * \brief Translates coordinates from the coordinate space of this window to the coordinate space of the monitor.
     * \param position coordinates in the space of this window
     * \return the coordinates in space of the monitor
     */
    [[nodiscard]] glm::ivec2 unmapPosition(const glm::ivec2& position);

    /**
     * \brief Translates coordinates from the monitor's coordinate space to the coordinate space of this window.
     * \param position the coordinate in screen space
     * \return coordinates in the space of this window
     */
    [[nodiscard]] glm::ivec2 mapPosition(const glm::ivec2& position);

    _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) override;

    void closeOverlappingSurfaceImpl(AOverlappingSurface* surface) override;
    virtual void onCloseButtonClicked();

signals:
    emits<> closed;
    emits<int, int> resized;
    emits<> redrawn;
    emits<> shown;

    /**
     * \brief Window is moving.
     * \param client area position.
     */
    emits<glm::vec2> moving;

    /**
     * \brief Window is maximized.
     */
    emits<> maximized;

    /**
     * \brief Window is minimized (hidden to the taskbar, iconified).
     */
    emits<> minimized;

    /**
     * \brief Window is restored (shown from the taskbar, deiconified).
     */
    emits<> restored;

    bool consumesClick(const glm::ivec2& pos) override;

    void onMouseMove(glm::ivec2 pos) override;

    void flagUpdateLayout() override;
};
