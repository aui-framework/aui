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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include "AUI/Views.h"
#include "AUI/Common/AString.h"

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"

#include "AUI/View/AViewContainer.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Util/AMetric.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(ANDROID)
#include <jni.h>
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

ENUM_FLAG(WindowStyle)
{
    WS_DEFAULT = 0,
	/**
	 * \brief Window without minimize and maximize buttons.
	 */
	WS_NO_MINIMIZE_MAXIMIZE = 0x1,

	/**
	 * \brief Disable window resize.
	 */
	WS_NO_RESIZE = 0x2,

	/**
	 * \brief Typical dialog window.
	 */
	WS_DIALOG = WS_NO_MINIMIZE_MAXIMIZE | WS_NO_RESIZE,

	/**
	 * \brief Remove standard window decorators.
	 */
	WS_NO_DECORATORS = 0x4,

	/**
	 * \brief Window for displaying system menu (dropdown, context menu)
	 */
	WS_SYS = 0x8
};

class API_AUI_VIEWS AWindow: public AViewContainer, public std::enable_shared_from_this<AWindow>
{
    friend class AWindowManager;
	friend struct painter;
private:
	static AWindow*& currentWindowStorage();
#if defined(_WIN32)
	HMODULE mInst;
	HDC mDC;
#else
    bool mWasMaximized = false;
#endif
    bool mRedrawFlag = true;
    bool mIsFocused = true;
	AString mWindowClass;
	AWindow* mParentWindow;
	float mDpiRatio = 1.f;

	/**
	 * \brief Handles self shared pointer.
	 */
	_<AWindow> mSelfHolder;

	struct Context
	{
#if defined(_WIN32)
		HGLRC hrc = 0;
#elif defined(ANDROID)
#else
        GLXContext context;
#endif

		~Context();
	};
	static Context context;

	AString mWindowTitle;
	
	_weak<AView> mFocusedView;

#if defined(_WIN32)
	friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#else
    unsigned long xGetWindowProperty(Atom property, Atom type, unsigned char** value) const;
    void xSendEventToWM(Atom atom, long a, long b, long c, long d, long e) const;
#endif

	void updateDpi();
	
protected:
#if defined(_WIN32)
	HWND mHandle;
	HICON mIcon = nullptr;
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(ANDROID)
    jobject mHandle = nullptr;
#else
    Window mHandle;
    XIC mIC;
#endif
    WindowStyle mWindowStyle = WS_DEFAULT;

	virtual void doDrawWindow();
	virtual void onClosed();
    void onCloseButtonClicked();
	
public:
	AWindow(const AString& name = "My window", int width = 854_dp, int height = 500_dp, AWindow* parent = nullptr, WindowStyle ws = WS_DEFAULT);
	virtual ~AWindow();

	void redraw();

    _<AView> determineSharedPointer() override;

    /**
     * \brief Checks whether last monitor frame is displayed and redraw will be efficient..
     *        If some object often updates UI thread for displaying some data it may cause extra CPU and GPU overload.
     *        AUI throttles window redraws and FPS does not go above 60 FPS but UI views may also cause extra CPU and
     *        GPU overload that does not have visual difference.
     * \return true if 16 milliseconds elapsed since last frame
     */
	static bool isRedrawWillBeEfficient();

	/**
	 * \return true if window is currently painting
	 */
	static bool isRenderingContextAcquired();

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

	void flagRedraw();
	void show();
	void close();
	void hide();

#if defined(_WIN32)
	HWND getNativeHandle() { return mHandle; }
#endif

	float getDpiRatio()
	{
		return mDpiRatio;
	}
	
	const AString& getWindowTitle() const
	{
		return mWindowTitle;
	}
	AWindowManager& getWindowManager() const;

	glm::ivec2 getWindowPosition() const;

    void setPosition(const glm::ivec2& position) override;
    void setSize(int width, int height) override;
    void setGeometry(int x, int y, int width, int height) override;


    void onMouseMove(glm::ivec2 pos) override;


	void onFocusAcquired() override;
	void onFocusLost() override;
	
	void onKeyDown(AInput::Key key) override;
	void onKeyRepeat(AInput::Key key) override;
	void onKeyUp(AInput::Key key) override;
	void onCharEntered(wchar_t c) override;
	void setFocusedView(const _<AView>& view);
	_<AView> getFocusedView() const
	{
		return mFocusedView.lock();
	}

    bool isFocused() const {
        return mIsFocused;
    }

    /**
     * \return Current window for current thread.
     */
	static AWindow* current();

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

signals:
	emits<> closed;
	emits<int, int> resized;
	emits<> dpiChanged;
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

	emits<AInput::Key> keyDown;

    void focusNextView();

    void onMousePressed(glm::ivec2 pos, AInput::Key button) override;

    bool consumesClick(const glm::ivec2& pos) override;
};
