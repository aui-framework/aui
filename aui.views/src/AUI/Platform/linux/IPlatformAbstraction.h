#pragma once

#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/AMessageBox.h"
/**
 * @brief Platform Abstraction Layer
 * @details
 * Runtime layer for switching between multiple displaying options (i.e, Linux = X11 or Wayland).
 *
 * This is a direct equivalent of Qt's Platform Abstraction.
 */
class IPlatformAbstraction {
public:
    IPlatformAbstraction();
    static IPlatformAbstraction& current();
    virtual ~IPlatformAbstraction() = default;

    virtual void init() = 0;

    // CURSOR
    virtual _<ACursor::Custom> createCustomCursor(AImageView image) = 0;
    virtual void applyNativeCursor(const ACursor& cursor, AWindow* pWindow) = 0;

    // CLIPBOARD
    virtual void copyToClipboard(const AString& text) = 0;
    virtual AString pasteFromClipboard() = 0;


    // DESKTOP
    virtual glm::ivec2 desktopGetMousePosition() = 0;
    virtual void desktopSetMousePosition(glm::ivec2 pos) = 0;

    virtual float platformGetDpiRatio() = 0;

    // INPUT
    virtual AInput::Key inputFromNative(int k) = 0;
    virtual int inputToNative(AInput::Key key) = 0;
    virtual bool inputIsKeyDown(AInput::Key k) = 0;

    // WINDOW
    virtual void windowSetStyle(AWindow& window, WindowStyle ws) = 0;
    virtual float windowFetchDpiFromSystem(AWindow& window) = 0;
    virtual void windowRestore(AWindow& window) = 0;
    virtual void windowMinimize(AWindow& window) = 0;
    virtual bool windowIsMinimized(AWindow& window) const = 0;
    virtual bool windowIsMaximized(AWindow& window) const = 0;
    virtual void windowMaximize(AWindow& window) = 0;
    virtual glm::ivec2 windowGetPosition(AWindow& window) const = 0;
    virtual void windowFlagRedraw(AWindow& window) = 0;
    virtual void windowShow(AWindow& window) = 0;
    virtual void windowSetSize(AWindow& window, glm::ivec2 size) = 0;
    virtual void windowSetGeometry(AWindow& window, int x, int y, int width, int height) = 0;
    virtual void windowSetIcon(AWindow& window, const AImage& image) = 0;
    virtual void windowHide(AWindow& window) = 0;
    virtual void windowManagerNotifyProcessMessages() = 0;
    virtual void windowManagerLoop() = 0;
    virtual void windowBlockUserInput(AWindow& window, bool blockUserInput) = 0;
    virtual void windowAllowDragNDrop(AWindow& window) = 0;
    virtual void windowShowTouchscreenKeyboardImpl(AWindow& window) = 0;
    virtual void windowHideTouchscreenKeyboardImpl(AWindow& window) = 0;
    virtual void windowMoveToCenter(AWindow& window) = 0;
    virtual void windowQuit(AWindow& window) = 0;
    virtual void windowAnnounceMinMaxSize(AWindow& window) = 0;
    virtual void windowManagerInitNativeWindow(const IRenderingContext::Init& init) = 0;
    virtual float windowGetDpiRatio(AWindow& window);

    // MESSAGE BOXES
    virtual AMessageBox::ResultButton messageBoxShow(
        AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon,
        AMessageBox::Button b) = 0;

    // some helper functions to pass through visibility.
    static void setCurrentWindow(AWindowBase* window);

    static bool& redrawFlag(AWindow& window) {
        return window.mRedrawFlag;
    }

    static AWindowNativePtr& nativeHandle(AWindow& window) {
        return window.mHandle;
    }

    static AWindow*& parentWindow(AWindow& window) {
        return window.mParentWindow;
    }
#if AUI_PLATFORM_LINUX
    static bool& wasMaximized(AWindow& window) {
        return window.mWasMaximized;
    }
#endif
};