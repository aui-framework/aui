#pragma once

#include "AUI/Platform/AWindow.h"
/**
 * @brief Platform Abstraction Layer
 * @details
 * Runtime layer for switching between multiple displaying options (i.e, Linux = X11 or Wayland).
 *
 * This is a direct equivalent of Qt's Platform Abstraction.
 */
class IPlatformAbstraction {
public:
    static IPlatformAbstraction& current();
    virtual ~IPlatformAbstraction() = default;
    virtual _<ACursor::Custom> createCustomCursor(AImageView image) = 0;
    virtual void applyNativeCursor(const ACursor& cursor, AWindow* pWindow) = 0;

    virtual void copyToClipboard(const AString& text) = 0;
    virtual AString pasteFromClipboard() = 0;

    virtual void windowQuit(AWindow& window) = 0;
    virtual void windowSetStyle(AWindow& window, WindowStyle ws) = 0;


    // DESKTOP
    virtual glm::ivec2 desktopGetMousePosition() = 0;
    virtual void desktopSetMousePosition(glm::ivec2 pos) = 0;

    virtual float platformGetDpiRatio() = 0;

    // INPUT
    virtual AInput::Key inputFromNative(int k) = 0;
    virtual int inputToNative(AInput::Key key) = 0;
    virtual bool inputIsKeyDown(AInput::Key k) = 0;

protected:
    void setCurrentWindow(AWindow* window);
};