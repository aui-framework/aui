#pragma once

#include "AUI/Views.h"
#include "AUI/Common/AString.h"

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"

#include "AUI/View/AViewContainer.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Util/AMetric.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(ANDROID)
#include <jni.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#endif

class Render;

ENUM_FLAG(WindowStyle)
{	
	/**
	 * \brief Окно без кнопок сворачивания и раскрытия.
	 */
	WS_SIMPLIFIED_WINDOW = 0x1,

	/**
	 * \brief Запретить изменять размер окна.
	 */
	WS_NO_RESIZE = 0x2,

	/**
	 * \brief Типичное диалоговое окно.
	 */
	WS_DIALOG = WS_SIMPLIFIED_WINDOW | WS_NO_RESIZE,

	/**
	 * \brief Без стандартных декораторов окна.
	 */
	WS_NO_DECORATORS = 0x4,
};

class TemporaryRenderingContext
{
	friend class AWindow;
private:
	_<struct painter> mPainter;
	explicit TemporaryRenderingContext(const _<painter>& painter)
		: mPainter(painter)
	{
	}
	
public:
};

class API_AUI_VIEWS AWindow: public AViewContainer, public IEventLoop, public std::enable_shared_from_this<AWindow>
{
	friend struct painter;
private:
	static AWindow*& currentWindowStorage();
#if defined(_WIN32)
	HMODULE mInst;
	HDC mDC;
#endif
	AString mWindowClass;
	AWindow* mParentWindow;
	float mDpiRatio = 1.f;

	/**
	 * \brief Удержание ссылки окна.
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
	bool mLoopRunning = false;
	
	_weak<AView> mFocusedView;

#if defined(_WIN32)
	friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	void updateDpi();
	
protected:
#if defined(_WIN32)
	HWND mHandle;
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(ANDROID)
    jobject mHandle;
#else
    Window mHandle;
    XIC mIC;
#endif

	virtual void doDrawWindow();
	virtual void onClosed();
	
public:
	AWindow(const AString& name = "My window", int width = 854, int height = 500, AWindow* parent = nullptr);
	virtual ~AWindow();

	void redraw();
	void loop();
	void quit();

	void setWindowStyle(WindowStyle ws);

	/**
	 * \brief Спрятать окно в панель задач.
	 */
	void minimize();
	void flagRedraw();
	void show();
	void close();

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

	glm::ivec2 getPos() const;

	TemporaryRenderingContext acquireTemporaryRenderingContext();

	void setSize(int width, int height) override;
	void onMouseMove(glm::ivec2 pos) override;


	void onFocusAcquired() override;
	void onFocusLost() override;
	
	void onKeyDown(AInput::Key key) override;
	void onKeyRepeat(AInput::Key key) override;
	void onKeyUp(AInput::Key key) override;
	void onCharEntered(wchar_t c) override;
	void setFocusedView(_<AView> view);
	_<AView> getFocusedView() const
	{
		return mFocusedView.lock();
	}

	void notifyProcessMessages() override;

	/**
	 * \brief Получить текущее окно для данного потока.
	 */
	static AWindow* current();

signals:
	emits<> closed;
	emits<int, int> resized;
	emits<> dpiChanged;
	emits<> redrawn;
	emits<> shown;

	emits<AInput::Key> keyDown;
};
