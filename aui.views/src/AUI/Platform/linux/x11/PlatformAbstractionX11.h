#pragma once

#include <AUI/Platform/linux/IPlatformAbstraction.h>
#include "AUI/Platform/ACursor.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
//#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/sync.h>


class PlatformAbstractionX11: public IPlatformAbstraction {
public:
    static void ensureXLibInitialized();
    static aui::assert_not_used_when_null<Display*> ourDisplay;
    static Screen* ourScreen;
    void init() override;

    static Window nativeHandle(AWindow& window) {
        return static_cast<Window>(window.getNativeHandle());
    }

    static void setNativeHandle(AWindow& window, Window x11Window) {
        IPlatformAbstraction::nativeHandle(window) = x11Window;
    }

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

    ~PlatformAbstractionX11() override = default;

    // CURSOR
    void applyNativeCursor(const ACursor &cursor, AWindow *pWindow) override;
    _<ACursor::Custom> createCustomCursor(AImageView image) override;

    // CLIPBOARD
    void copyToClipboard(const AString& text) override;
    AString pasteFromClipboard() override;

    // INPUT
    AInput::Key inputFromNative(int k) override;
    int inputToNative(AInput::Key key) override;
    bool inputIsKeyDown(AInput::Key k) override;

    // PLATFORM/DESKTOP
    glm::ivec2 desktopGetMousePosition() override;
    void desktopSetMousePosition(glm::ivec2 pos) override;


    // WINDOW
    void windowSetStyle(AWindow& window, WindowStyle ws) override;
    float windowFetchDpiFromSystem(AWindow& window) override;
    void windowRestore(AWindow& window) override;
    void windowMinimize(AWindow& window) override;
    bool windowIsMinimized(AWindow& window) const override;
    bool windowIsMaximized(AWindow& window) const override;
    void windowMaximize(AWindow& window) override;
    glm::ivec2 windowGetPosition(AWindow& window) const override;
    void windowFlagRedraw(AWindow& window) override;
    void windowShow(AWindow& window) override;
    void windowSetSize(AWindow& window, glm::ivec2 size) override;
    void windowSetGeometry(AWindow& window, int x, int y, int width, int height) override;
    void windowSetIcon(AWindow& window, const AImage& image) override;
    void windowHide(AWindow& window) override;
    void windowManagerNotifyProcessMessages() override;
    void windowManagerLoop() override;
    void windowBlockUserInput(AWindow& window, bool blockUserInput) override;
    void windowAllowDragNDrop(AWindow& window) override;
    void windowShowTouchscreenKeyboardImpl(AWindow& window) override;
    void windowHideTouchscreenKeyboardImpl(AWindow& window) override;
    void windowMoveToCenter(AWindow& window) override;
    void windowQuit(AWindow& window) override;
    void windowAnnounceMinMaxSize(AWindow& window) override;
    void windowManagerInitNativeWindow(const IRenderingContext::Init& init) override;
    AMessageBox::ResultButton messageBoxShow(
        AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
        AMessageBox::Button b) override;

private:
    void xProcessEvent(XEvent& ev);
    void xClipboardClear();
    void xHandleClipboard(const XEvent& ev);

    Pipe mNotifyPipe;
    std::atomic_bool mFastPathNotify = false;
};