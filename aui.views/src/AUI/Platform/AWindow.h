/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/Views.h"
#include "AUI/Common/AString.h"

#include "AUI/Common/AObject.h"
#include "AUI/Common/ASignal.h"

#include "AUI/Platform/ASurface.h"
#include "AUI/Thread/IEventLoop.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AProperty.h"
#include "AWindowNativePtr.h"
#include <AUI/Enum/AScreenOrientation.h>
#include <variant>

#if AUI_PLATFORM_WIN
#include <Windows.h>
#include <AUI/Platform/win32/Win32Util.h>

#elif AUI_PLATFORM_ANDROID
#include <jni.h>
#elif AUI_PLATFORM_APPLE
#else

#endif

class ARender;
class AWindowManager;
class ATitlebar;

/**
 * @brief Space occupied by the native window controls (traffic lights, close/minimize/maximize buttons) inside the
 *        window, in pixels.
 * @details
 * Relevant for AWindow::Init::DecorationExtended only: the window contents extend into the titlebar, and the native
 * controls are drawn over them. ATitlebar uses this to keep its contents out of the controls' way.
 */
struct AWindowControlsInsets {
    /**
     * @brief Space occupied by native controls at the left edge of the window.
     */
    int left = 0;

    /**
     * @brief Space occupied by native controls at the right edge of the window.
     */
    int right = 0;

    /**
     * @brief Height of the native controls.
     */
    int height = 0;

    bool operator==(const AWindowControlsInsets&) const noexcept = default;
};

/**
 * @brief Represents a window in the underlying windowing system.
 * @ingroup views
 * @details
 * This document covers AUI's window management system, which provides cross‑platform
 * window creation, lifecycle management, and event handling. The system abstracts
 * native windowing APIs across Windows, Linux, macOS, Android, and iOS platforms
 * while maintaining a unified interface for application developers.
 *
 * `AWindow` is the concrete implementation of a UI surface that is backed by a native
 * window. It inherits from `ASurface` – the generic surface API that can be used by other windowing API implementations
 * (i.e., SDL3 or a game engine).
 *
 * `AWindow` owns an `IRenderingContext` instance. The context bridges the window to
 * the shared `IRenderer` – the actual rendering engine. The renderer performs all
 * drawing primitives and is shared across all windows. The context handles
 * platform‑specific tasks such as buffer swapping, resizing, and screenshot capture.
 *
 * ```mermaid
 * graph TD
 * %%{init: {'theme': 'default'}}%%
 * classDef platformSpec fill:none,stroke:#888,stroke-width:2,stroke-dasharray:5,5;
 *
 * %% Sub‑graph definition (use an ID without spaces)
 * subgraph PlatformSpecifics
 *     AWindow1(AWindow) -->|has| IRenderingContext1(IRenderingContext)
 *     AWindow2(AWindow) -->|has| IRenderingContext2(IRenderingContext)
 *     AWindow3(AWindow) -->|has| IRenderingContext3(IRenderingContext)
 * end
 *
 * %% Apply the class to the sub‑graph
 * class PlatformSpecifics platformSpec
 *
 * %% Connections to the renderer
 * IRenderingContext1 -->|uses| IRenderer
 * IRenderingContext2 -->|uses| IRenderer
 * IRenderingContext3 -->|uses| IRenderer
 * ```
 *
 * For more details on the rendering, see [Rendering Engine](irenderer.md).
 */
class API_AUI_VIEWS AWindow: public ASurface
{
    friend class OpenGLRenderingContext;
    friend class IPlatformAbstraction;
    friend class CommonRenderingContext;
    friend class SoftwareRenderingContext;
    friend class AWindowManager;
    friend struct painter;
public:
    /**
     * @brief Window creation parameters.
     * @details
     * Everything here except `title` and `resizable` is applied at window creation and can't be changed afterwards.
     * `title` and `resizable` are the initial values; use AWindow::setTitle and AWindow::setResizable to change them
     * at runtime.
     *
     * ```cpp
     * _new<AWindow>(AWindow::Init {
     *     .title = "Settings",
     *     .width = 400_dp,
     *     .height = 300_dp,
     *     .parent = AWindow::Init::Parent { .window = *this, .modal = true },
     *     .decoration = AWindow::Init::DecorationDefault { .buttons = { .minimize = false, .maximize = false } },
     *     .resizable = false,
     * });
     * ```
     *
     * @specificto{desktop} `decoration`, `parent` and `resizable` are ignored on Android, iOS and web: the window is the
     * surface of the application there.
     */
    struct Init {
        /**
         * @brief Which window controls (buttons) to show.
         */
        struct Buttons {
            bool minimize = true;
            bool maximize = true;
            bool close = true;
        };

        /**
         * @brief Ordinary OS-drawn titlebar and border.
         */
        struct DecorationDefault {
            Buttons buttons;
        };

        /**
         * @brief Window contents extend into the titlebar.
         * @details
         * Native window controls are kept where the platform has them (traffic lights on macOS, window controls on
         * GTK) and reserve space via ATitlebar; on the other platforms AUI draws them. Use ATitlebar to fill the
         * titlebar.
         */
        struct DecorationExtended {
            Buttons buttons;
        };

        /**
         * @brief No titlebar, no buttons.
         * @details
         * Native border, shadow and rounded corners are kept when `shadow` is true and the platform is able to. Good
         * for splash screens.
         */
        struct DecorationBorderless {
            bool shadow = true;
        };

        using Decoration = std::variant<DecorationDefault, DecorationExtended, DecorationBorderless>;

        /**
         * @brief Parent window.
         * @details
         * A modal window blocks user input of its parent until closed. Modality without a parent is not expressible.
         */
        struct Parent {
            AWindow& window;
            bool modal = false;
        };

        AString title = "My window";
        AMetric width = 854_dp;
        AMetric height = 500_dp;
        AOptional<Parent> parent;
        Decoration decoration = DecorationDefault {};
        bool resizable = true;
    };

    AWindow();
    explicit AWindow(Init init);
    AWindow(AString title, AMetric width = 854_dp, AMetric height = 500_dp);
    ~AWindow() override;

    [[nodiscard]]
    unsigned frameMillis() const noexcept override {
        return mFrameMillis;
    }

    void redraw() override;

    void blockUserInput(bool blockUserInput = true) override;

    /**
     * @return true if the user input is blocked by AWindow::blockUserInput (i.e., a modal child window is open).
     */
    [[nodiscard]]
    bool isUserInputBlocked() const noexcept { return mInputBlocked; }

    /**
     * @brief Checks whether last monitor frame is displayed and redraw will be efficient.
     *        If some object often updates UI thread for displaying some data it may cause extra CPU and GPU overload.
     *        AUI throttles window redraws and FPS does not go above 60 FPS but UI views may also cause extra CPU and
     *        GPU overload that does not have visual difference.
     * @return true if 16 milliseconds elapsed since last frame
     */
    static bool isRedrawWillBeEfficient();

    void setIcon(const AImage& image);

    /**
     * @return parameters this window was created with.
     * @details
     * Read only. `Init::title` and `Init::resizable` are the initial values only; see AWindow::title and
     * AWindow::isResizable for actual ones.
     */
    [[nodiscard]]
    const Init& init() const noexcept { return mInit; }

    /**
     * @return parent window, if any.
     */
    [[nodiscard]]
    AWindow* parentWindow() const noexcept { return mInit.parent ? &mInit.parent->window : nullptr; }

    /**
     * @brief Popup windows are the ones that host dropdowns and context menus. They are created by AUI only.
     */
    [[nodiscard]]
    bool isPopup() const noexcept { return mIsPopup; }

    /**
     * @brief Changes window title.
     */
    void setTitle(AString title);

    /**
     * @return current window title.
     */
    [[nodiscard]]
    const AString& title() const noexcept { return mWindowTitle; }

    /**
     * @brief Allows or forbids the user to resize the window.
     * @specificto{desktop}
     */
    void setResizable(bool resizable);

    /**
     * @return whether the user is able to resize the window. Reads the state of the native window.
     */
    [[nodiscard]]
    bool isResizable() const;

    /**
     * @brief Space occupied by the native window controls, in pixels.
     * @details
     * Non-zero only for AWindow::Init::DecorationExtended on platforms which draw the window controls natively
     * (macOS, GTK).
     */
    AProperty<AWindowControlsInsets> windowControlsInsets;

    /**
     * @return true if the platform draws the window controls (traffic lights, etc.) of AWindow::Init::DecorationExtended
     * windows itself; they occupy AWindow::windowControlsInsets.
     */
    [[nodiscard]]
    bool hasNativeWindowControls() const;

    /**
     * @brief Whether the specified point (in window coordinates) belongs to the titlebar, i.e., can be used to drag the
     *        window.
     * @details
     * A point is a caption if the deepest view there is inside ATitlebar and is neither an [AButton] nor has
     * `.override-title-dragging` ASS class.
     */
    [[nodiscard]]
    bool isCaptionAt(const glm::ivec2& pos);

    /**
     * @brief Registers the titlebar of this window. Called by ATitlebar.
     */
    void setTitlebar(ATitlebar* titlebar) noexcept { mTitlebar = titlebar; }

    [[nodiscard]]
    ATitlebar* titlebar() const noexcept { return mTitlebar; }

    /**
     * @brief Minimizes window (hide window to the taskbar, iconifies)
     */
    void minimize();

    /**
     * @return true if window minimized (hidden in taskbar, iconified)
     */
    bool isMinimized() const;

    /**
     * @brief Maximizes window (makes window fullscreen)
     */
    void maximize();

    /**
     * @return true if window maximized (fullscreen)
     */
    bool isMaximized() const;

    /**
     * @brief Restores window (shows window from taskbar)
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

    /**
     * @brief Closes the window.
     * @details
     * Closing is terminal and works the same on all platforms: `closed` is emitted exactly once, the modal parent is
     * unblocked, the native window is destroyed and the window is removed from the AWindowManager. The window can't be
     * shown again after that; create another one. Calling close() on a closed window does nothing.
     *
     * Use AWindow::hide to hide the window temporarily.
     */
    void close();

    /**
     * @brief Hides the window without closing it.
     * @details
     * A hidden window is still alive: it is counted as an open window, so it keeps the application running
     * (see AApplication::quitOnLastWindowClosed). Use AWindow::close to destroy it.
     *
     * A hidden window is not visible anywhere, including the taskbar.
     */
    void hide();

    /**
     * @brief Shows the window, restores it if minimized and brings it to foreground.
     * @param activationToken platform-specific focus stealing permission token (see AActivation::activationToken).
     * @details
     * Typical usage is handling AApplication::activated:
     * ```cpp
     * connect(AApplication::inst().activated, [window](const AActivation& a) { window->activate(a.activationToken); });
     * ```
     */
    void activate(const AString& activationToken = {});

    [[nodiscard]]
    bool isClosed() const noexcept;

#if AUI_PLATFORM_WIN
    HWND getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_ANDROID
    jobject getNativeHandle() { return mHandle; }
#elif AUI_PLATFORM_APPLE
#else
    AWindowNativePtr getNativeHandle() { return mHandle; }
#endif

    auto nativeHandle() const {
        return mHandle;
    }

    glm::ivec2 getWindowPosition() const;

    /**
     * @brief The smallest size this window can be resized to.
     * @details
     * In contrast to AView::getMinSize, which is the explicitly styled minimum, this one is measured from the
     * window's contents: a window is never smaller than the space its contents need at their narrowest, so its
     * layout is never squeezed into nothing. The styled minimum is respected as well, and the result is capped by
     * AWindow::MAX_MIN_SIZE.
     */
    glm::ivec2 getMinimumSize();

    void setPosition(glm::ivec2 position);
    void setSize(glm::ivec2 size);
    void setGeometry(int x, int y, int width, int height);

    void onFocusAcquired() override;
    void onFocusLost() override;

    void onKeyDown(AInput::Key key) override;
    void onKeyRepeat(AInput::Key key) override;

    void setFocusNextViewOnTab(bool value) {
        mFocusNextViewOnTab = value;
    }

    /**
     * @brief Wraps your AView to window.
     * @param view view to wrap
     * @param init window creation parameters
     * @return created window, AWindow::show() is not called
     */
    static _<AWindow> wrapViewToWindow(const _<AView>& view, Init init);

    [[deprecated("Use ASurface::current()")]]
    static ASurface* current() {
      return ASurface::current();
    }

    /**
     * @brief Translates coordinates from the coordinate space of this window to the coordinate space of another window.
     * @param position coordinates in the space of this window
     * @param other other window
     * @return coordinates in the space of the other window
     */
    [[nodiscard]] glm::ivec2 mapPositionTo(const glm::ivec2& position, _<AWindow> other);

    /**
     * @brief Translates coordinates from the coordinate space of this window to the coordinate space of the monitor.
     * @param position coordinates in the space of this window
     * @return the coordinates in space of the monitor
     */
    [[nodiscard]] glm::ivec2 unmapPosition(const glm::ivec2& position);

    /**
     * @brief Translates coordinates from the monitor's coordinate space to the coordinate space of this window.
     * @param position the coordinate in screen space
     * @return coordinates in the space of this window
     */
    [[nodiscard]] glm::ivec2 mapPosition(const glm::ivec2& position);

    _<AOverlappingSurface> createOverlappingSurfaceImpl(const glm::ivec2& position, const glm::ivec2& size) override;

    void closeOverlappingSurfaceImpl(_<AOverlappingSurface> surface) override;

    /**
     * @brief The user clicked the close button.
     * @details
     * Closes the window by default. Override to refuse closing.
     */
    virtual void onCloseButtonClicked();

    void forceUpdateCursor() override;

    void showTouchscreenKeyboardImpl() override;
    void hideTouchscreenKeyboardImpl() override;

    /**
     * @brief Moves the window to the center of monitor.
     * @details
     * When using in series with setSize(), do the setSize() first, when moveToCenter().
     */
    void moveToCenter();

    /**
     * @brief Controls mobile device's screen orientation when this window is on the foreground.
     * @details
     * Affects only mobile OSes. On window-based interfaces (desktop) does nothing.
     */
    void setMobileScreenOrientation(AScreenOrientation screenOrientation);

signals:
    emits<> closed;
    emits<> shown;

    /**
     * @brief Window is moving.
     * @param client area position.
     */
    emits<glm::vec2> moving;

    /**
     * @brief Window is maximized.
     */
    emits<> maximized;

    /**
     * @brief Window is minimized (hidden to the taskbar, iconified).
     */
    emits<> minimized;

    /**
     * @brief Window is restored (shown from the taskbar, deiconified).
     */
    emits<> restored;

    bool consumesClick(const glm::ivec2& pos) override;

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerPressed(const APointerPressedEvent& event) override;

protected:
#if AUI_PLATFORM_WIN
    aui::win32::Icon mIcon;
    virtual LRESULT winProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_APPLE
#endif
    AWindowNativePtr mHandle = 0; // on linux AWindowNativePtr is not a pointer type so using zero here

    virtual void doDrawWindow();

    struct PopupTag {};

    /**
     * @brief Constructs a popup window (dropdown, context menu).
     */
    AWindow(PopupTag, AWindow& parent);

    void windowNativePreInit();

    /**
     * @brief Enables drag-n-drop for this window.
     */
    void allowDragNDrop();

    void createDevtoolsWindow() override;

    float fetchDpiFromSystem() const override;

    /**
     * @brief defines if the next view must be focused on tab button pressed
     */
    bool mFocusNextViewOnTab = false;

private:
#if AUI_PLATFORM_WIN

#elif AUI_PLATFORM_ANDROID
#elif AUI_PLATFORM_LINUX
    bool mWasMaximized = false;
#endif
#ifdef AUI_PLATFORM_MACOS
    bool mRedrawFlag = false;
#else
    bool mRedrawFlag = true;
#endif
    AString mWindowClass;
    Init mInit;
    bool mIsPopup = false;
    ATitlebar* mTitlebar = nullptr;

    /**
     * @brief close() was called.
     */
    bool mClosed = false;

    /**
     * @brief User input is blocked (a modal child window is open).
     */
    bool mInputBlocked = false;

    /**
     * @brief Window manager moves or resizes the window by our request.
     */
    bool mWindowManagerDragging = false;

    /**
     * @brief The native window was destroyed.
     */
    bool mNativeDestroyed = false;

    unsigned mFrameMillis = 1;

    /**
     * @brief Handles self shared pointer.
     */
    _<AWindow> mSelfHolder;

    _<void> mLifetimeToken = _new<char>();

    AString mWindowTitle;

    // platform-specific parts

    /**
     * @brief Handles a pointer press on the titlebar or window edge by the means of the platform (window drag, resize,
     *        maximize on double click).
     * @return true if the event is consumed.
     */
    bool captionPressed(const APointerPressedEvent& event);
#if AUI_PLATFORM_LINUX
    std::chrono::steady_clock::time_point mLastCaptionPress {};
    void applyResizeEdgeCursor(glm::vec2 pos);
#endif
#if AUI_PLATFORM_MACOS
    void applyTitlebarRect();
    void updateWindowControlsInsets();
#endif
    void showImpl();
    void hideImpl();
    void setTitleImpl(const AString& title);
    void destroyNativeWindow();

#if AUI_PLATFORM_WIN
    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
#if AUI_PLATFORM_LINUX
    unsigned long xGetWindowProperty(unsigned long property, unsigned long type, unsigned char** value) const;
    void xSendEventToWM(unsigned long atom, long a, long b, long c, long d, long e) const;
#endif

};
