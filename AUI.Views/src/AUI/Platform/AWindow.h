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
#include <X11/Xatom.h>
#endif

class Render;
class AWindowManager;

ENUM_FLAG(WindowStyle)
{
    WS_DEFAULT = 0,
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

	/**
	 * \brief Окно для вывода системного меню (выпадающий список дропбокса)
	 */
	WS_SYS = 0x8
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

class API_AUI_VIEWS AWindow: public AViewContainer, public std::enable_shared_from_this<AWindow>
{
    friend class AWindowManager;
	friend struct painter;
private:
	static AWindow*& currentWindowStorage();
#if defined(_WIN32)
	HMODULE mInst;
	HDC mDC;
#endif
    bool mRedrawFlag = true;
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
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(ANDROID)
    jobject mHandle = nullptr;
#else
    Window mHandle;
    XIC mIC;
#endif

	virtual void doDrawWindow();
	virtual void onClosed();
    void onCloseButtonClicked();
	
public:
	AWindow(const AString& name = "My window", int width = 854, int height = 500, AWindow* parent = nullptr, WindowStyle ws = WS_DEFAULT);
	virtual ~AWindow();

	void redraw();

	/**
	 * \brief Удаляет окно из AWindowManager.
	 */
	void quit();

	void setWindowStyle(WindowStyle ws);

	/**
	 * \brief Спрятать окно в панель задач.
	 */
	void minimize();

	/**
	 * \return true, если окно спрятано в панель задач
	 */
	bool isMinimized() const;

	/**
	 * \brief Развернуть окно на весь экран.
	 */
	void maximize();

	/**
	 * \return true, если окно раскрыто на весь экран
	 */
	bool isMaximized() const;

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
	AWindowManager& getWindowManager() const;

	glm::ivec2 getWindowPosition() const;

	TemporaryRenderingContext acquireTemporaryRenderingContext();

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
	void setFocusedView(_<AView> view);
	_<AView> getFocusedView() const
	{
		return mFocusedView.lock();
	}

	/**
	 * \brief Получить текущее окно для данного потока.
	 */
	static AWindow* current();

	/**
	 * \brief Перевести координаты из координатного пространства этого окна в координатное пространство другого окна.
	 * \param position координаты в пространстве этого окна
	 * \param other другое окно
	 * \return координаты в пространстве окна other
	 */
	[[nodiscard]] glm::ivec2 mapPositionTo(const glm::ivec2& position, _<AWindow> other);

    /**
     * \brief Перевести координаты из координатного пространства этого окна в координатное пространство монитора.
     * \param position координаты в пространстве этого окна
     * \return координаты в пространстве монитора
     */
    [[nodiscard]] glm::ivec2 unmapPosition(const glm::ivec2& position);

    /**
     * \brief Перевести координаты из координатного пространства монитора в координатное пространство этого окна.
     * \param position координаты в пространстве монитора
     * \return координаты в пространстве этого окна
     */
    [[nodiscard]] glm::ivec2 mapPosition(const glm::ivec2& position);

signals:
	emits<> closed;
	emits<int, int> resized;
	emits<> dpiChanged;
	emits<> redrawn;
	emits<> shown;

	emits<AInput::Key> keyDown;

};
