#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xatom.h>

#include <AUI/Platform/linux/IPlatformAbstraction.h>
#include "AUI/Platform/ACursor.h"


class PlatformAbstractionX11: public IPlatformAbstraction {
public:
    static void ensureXLibInitialized();
    static aui::assert_not_used_when_null<Display*> ourDisplay;
    static Screen* ourScreen;

    static Window nativeHandle(AWindow& window) {
        return static_cast<Window>(window.getNativeHandle());
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

    void applyNativeCursor(const ACursor &cursor, AWindow *pWindow) override;
    _<ACursor::Custom> createCustomCursor(AImageView image) override;
    void copyToClipboard(const AString& text) override;
    AString pasteFromClipboard() override;

    void windowSetStyle(AWindow& window, WindowStyle ws) override;
    void windowQuit(AWindow& window) override;

    glm::ivec2 desktopGetMousePosition() override;
    void desktopSetMousePosition(glm::ivec2 pos) override;

private:
    void xProcessEvent(XEvent& ev);
    void xClipboardClear();
    void xHandleClipboard(const XEvent& ev);
    void setCurrentWindow(AWindow* window);
};