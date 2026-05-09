#pragma once

#include <AUI/Platform/APlatform.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AMessageBox.h>

/**
 * @brief Platform Abstraction Layer
 * @details
 * Runtime layer for switching between multiple displaying options (i.e, Linux = X11 or Wayland).
 *
 * This is a direct equivalent of Qt's Platform Abstraction.
 *
 * @exclusivefor{linux} Platform Abstraction Layer is used to choose appropriate core platform UI APIs in runtime.
 *
 * AUI framework is designed to be flexible and adaptable to different desktop environments and UI toolkits. This is
 * achieved through a platform abstraction layer that isolates the core application logic from the specifics of the
 * underlying UI implementation.
 *
 * ## Supported UI Backends
 *
 * Currently, the framework supports the following UI backends:
 * - **Adwaita:** modern UI toolkit based on GTK, commonly used in GNOME.
 * - **GTK:** widely available widget toolkit.
 * - **X11:** legacy windowing system (used as a fallback).
 *
 * ## API Selection and Prioritization
 *
 * The framework determines the preferred UI backend based on an `initializationOrder` defined within
 * [APlatformAbstractionOptions]. The order prioritizes backends, attempting to initialize them sequentially.
 *
 * ## Dynamic Loading
 *
 * To minimize dependencies and enhance compatibility, the framework utilizes dynamic loading for certain UI backends.
 * For example, the Adwaita backend loads the `libadwaita-1.so` library at runtime using `dlopen`.
 *
 * The `generate_dl_subs` function from `aui.toolbox` is used to automatically generate `dlsym`-based function
 * implementations based on a list of signatures (`*.sigs`) that need to be exported from the dynamically loaded
 * libraries. This allows the framework to call functions within the loaded library without requiring explicit linking.
 */
class IPlatformAbstraction : public APlatform {
public:
    static std::unique_ptr<IPlatformAbstraction> create();
    static IPlatformAbstraction& current();
    static IPlatformAbstraction* currentSafe();

    IPlatformAbstraction();
    ~IPlatformAbstraction() override = default;

    virtual void init() = 0;

    // CURSOR
    virtual _<ACursor::Custom> createCustomCursor(AImageView image) = 0;
    virtual void applyNativeCursor(const ACursor& cursor, AWindow* pWindow) = 0;

    // DESKTOP
    virtual glm::ivec2 desktopGetMousePosition() = 0;
    virtual void desktopSetMousePosition(glm::ivec2 pos) = 0;

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

    // some helper functions to pass through visibility.
    static void setCurrentWindow(ASurface* window);

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