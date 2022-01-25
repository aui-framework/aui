#pragma once

#include "IRenderingContext.h"
#include "AWindow.h"
#include "AUI/Traits/values.h"

class CommonRenderingContext: public IRenderingContext {
protected:
#if AUI_PLATFORM_WIN

    AString mWindowClass;

    /**
     * GetDC() HDC
     */
    HDC mWindowDC;

    /**
     * BeginPaint() HDC
     */
    HDC mPainterDC = nullptr;

    PAINTSTRUCT mPaintstruct;
#endif
#if AUI_PLATFORM_LINUX
    void initX11Window(const Init& init, XSetWindowAttributes& swa, XVisualInfo* vi);
#endif
#if AUI_PLATFORM_MACOS
    AWindow* mWindow;
    void* mDisplayLink;
#endif
public:
#if AUI_PLATFORM_LINUX
    static void ensureXLibInitialized();
    static aui::assert_not_used_when_null<Display*> ourDisplay;
    static Screen* ourScreen;

    static struct Atoms {
        Atom wmProtocols;
        Atom wmDeleteWindow;
        Atom wmHints;
        Atom wmState;
        Atom netWmState;
        Atom netWmStateMaximizedVert;
        Atom netWmStateMaximizedHorz;
        Atom clipboard;
        Atom utf8String;
        Atom textPlain;
        Atom textPlainUtf8;
        Atom auiClipboard;
        Atom incr;
        Atom targets;
        Atom netWmSyncRequest;
        Atom netWmSyncRequestCounter;
    } ourAtoms;
#endif
#if AUI_PLATFORM_MACOS
    void requestFrame();
#endif

    void init(const Init& init) override;

    void beginPaint(ABaseWindow& window) override;

    void endPaint(ABaseWindow& window) override;

    ~CommonRenderingContext() override = default;

    void destroyNativeWindow(ABaseWindow& window) override;
};