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
#include "AUI/Common/ABoxFields.h"
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
class IDrawable;

/**
 * @brief Space occupied by the window controls (traffic lights, close/minimize/maximize buttons) inside the
 *        window, in pixels.
 * @details
 * Relevant for `AWindow::Init::DecorationExtended` only: the window contents extend into the titlebar, and the controls
 * are drawn over them. The controls are reported regardless of who draws them: the OS (macOS, GTK) or AUI (Windows,
 * X11), so the value means the same on every platform. Titlebar uses this to keep its contents out of the controls'
 * way.
 */
struct AWindowControlsInsets {
    /**
     * @brief Space occupied by the controls at the left edge of the window.
     */
    int left = 0;

    /**
     * @brief Space occupied by the controls at the right edge of the window.
     */
    int right = 0;

    /**
     * @brief Height of the controls.
     */
    int height = 0;

    bool operator==(const AWindowControlsInsets&) const noexcept = default;
};

/**
 * @brief Represents a window in the underlying windowing system.
 * @ingroup views
 * @details
 * ## Opening a window
 *
 * Create an AWindow, put views into it with `AWindow::setContents` and call `AWindow::show`:
 *
 * ```cpp
 * #include <AUI/Platform/Entry.h>
 * #include <AUI/Platform/AWindow.h>
 * #include <AUI/View/ALabel.h>
 *
 * AUI_ENTRY {
 *     auto window = _new<AWindow>("Hello AUI", 300_dp, 200_dp);
 *     window->setContents(Centered { Label { "Hello, world!" } });
 *     window->show();
 *     return 0;
 * }
 * ```
 *
 * Every window implicitly holds the application (see [AApplication::Hold]), so by default the application keeps
 * running while at least one window is alive. This is adjustable: create an explicit hold with
 * [AApplication::hold()] to keep the application running after the last window is closed, or release it to allow the
 * application to exit. The window is owned by AUI while it is shown, so
 * you don't have to store the pointer; store it only if you want to talk to the window later.
 *
 * To open another window from the application (i.e., a settings dialog), do the same from any slot or callback on the
 * UI thread:
 *
 * ```cpp
 * connect(settingsButton->clicked, this, [this] {
 *     auto settings = _new<AWindow>("Settings", 400_dp, 300_dp);
 *     settings->setContents(Vertical { Label { "Settings go here" } });
 *     settings->show();
 * });
 * ```
 *
 * `AWindow(title, width, height)` creates an ordinary window with the OS-drawn titlebar. Anything else (a parent
 * window, modality, custom titlebar, fixed size) is configured with `AWindow::Init`, see [below](#creation-decorations-and-modality).
 * To close the window from code, use `AWindow::close`.
 *
 * ## Architecture
 *
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
 *
 *
 * ## Creation, decorations and modality
 *
 * AWindow is created with `AWindow::Init` - an aggregate describing everything that can't be changed afterwards:
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
 * Meaningless combinations do not compile: modality exists only as a part of `parent`, window buttons exist only in
 * decorations which have them.
 *
 * | Decoration | What it is | Replaces |
 * |---|---|---|
 * | `DecorationDefault { buttons }` | Ordinary OS-drawn titlebar and border. | `WindowStyle::DEFAULT`, `NO_MINIMIZE_MAXIMIZE` |
 * | `DecorationExtended { buttons }` | Window contents extend into the titlebar. Native window controls are kept where the platform has them. | `ACustomCaptionWindow` |
 * | `DecorationBorderless { shadow }` | No titlebar, no buttons. Native border, shadow and rounded corners are kept where possible. | `ACustomWindow`, `NO_DECORATORS` |
 *
 * A modal window blocks user input of its parent until closed. Having `parent` without `modal` produces an ordinary child
 * window - it does not block anything, even when it is not resizable.
 *
 * ## Titlebar of Extended and Borderless windows
 *
 * [declarative::Titlebar] is a view you place into the window contents yourself; it is not created by the window. It is both a
 * *drag handle* and a *layout container for the titlebar contents*. What it does depends on the decoration:
 *
 * | | `DecorationExtended` | `DecorationBorderless` |
 * |---|---|---|
 * | Drags the window | yes | yes |
 * | Double click maximizes/restores | yes | yes |
 * | Window controls (close, minimize, maximize) | yes: native or drawn by AUI, according to `buttons` | no, [declarative::Titlebar] is just your contents |
 * | Space reserved for native controls | yes ([AWindow::windowControlsInsets]) | no, zero |
 *
 * Without [declarative::Titlebar] a Borderless window can't be moved by the user at all, and an Extended window has no draggable area.
 *
 * What can be put into [declarative::Titlebar] of a Borderless window: any view. Contents are laid out horizontally, the first and
 * only child stretches over the full width. Put a label for a plain caption, or a `Horizontal` with a logo, title, search
 * field, tool buttons and so on:
 *
 * ```cpp
 * auto w = _new<AWindow>(AWindow::Init {
 *     .title = "Custom window without caption",
 *     .width = 400_dp,
 *     .height = 300_dp,
 *     .decoration = AWindow::Init::DecorationBorderless {},
 * });
 * w->setContents(Vertical {
 *     // an opaque caption strip: the window is dragged by it
 *     Titlebar { Label { "Drag me" } AUI_OVERRIDE_STYLE { Padding { 4_dp } } },
 *     Centered { Label { "Window contents" } } AUI_LET { it->setExpanding(); },
 * });
 * w->show();
 * ```
 *
 * Nothing is drawn by the window itself, so everything about the look (background, height, padding, border) is up to you
 * and set by ASS on [declarative::Titlebar] or its contents. Interactive views inside the titlebar work as usual: buttons
 * ([AButton]) and views with `.override-title-dragging` ASS class are not drag areas, everything else under [declarative::Titlebar]
 * (labels, spacers, empty space, text fields included) starts window dragging.
 *
 * With `DecorationExtended` the window contents extend under the system titlebar, so [declarative::Titlebar] is typically overlaid on
 * top of the contents (see [example](custom-window-titlebar-blur.md) for a complete example with a blurred backdrop):
 *
 * ```cpp
 * setContents(Stacked {
 *     content,
 *     Vertical {
 *         Titlebar { Horizontal { Label { "My app" } } },
 *         SpacerExpanding {},
 *     },
 * });
 * ```
 *
 * ### Window title inside the titlebar
 *
 * You don't have to imitate the title of each OS yourself. Put [declarative::WindowTitle] into
 * [declarative::Titlebar] (`Titlebar {}` without contents contains it) to mark where the title is, and the OS draws it
 * there, the same way it does in its own titlebars, wherever it can:
 *
 * | | Who draws the title | Where |
 * |---|---|---|
 * | macOS | AppKit (the native title) | centered over the window horizontally, centered in [declarative::Titlebar] vertically |
 * | Linux GTK/adw | GTK (the native header bar title) | like in a header bar: centered over the window while it fits into [declarative::WindowTitle], otherwise shifted into it |
 * | Windows | AUI: DWM does not draw the caption text of windows with the extended client area | left-aligned label |
 * | Linux X11, Borderless windows | AUI | centered label |
 *
 * The native title follows the system font, color and the inactive window look. It is drawn over the window contents,
 * so neither ASS nor other views can affect it; it is shown only while [declarative::WindowTitle] is present (see
 * `AWindow::hasNativeWindowTitle`). Where AUI draws the title, style it with `t<AWindowTitle>()`.
 *
 * The result looks like an ordinary titlebar, but you control its background and can place your own views around
 * the title:
 *
 * ```cpp
 * setContents(Stacked {
 *     content,
 *     Vertical {
 *         Titlebar {},   // looks like the OS titlebar
 *         SpacerExpanding {},
 *     },
 * });
 * ```
 *
 * The window is the single source of truth for the title: `AWindow::title` is a property; the native title is the
 * window title itself, and the AUI-drawn [declarative::WindowTitle] is bound to it. Change the title with `AWindow::setTitle` (or `title() = ...`), and the
 * taskbar, window switcher and the title in your titlebar update together. Do not duplicate the title in a `Label`
 * with a string literal. If you need a label of your own (for example, with a prefix), bind it to the property:
 *
 * ```cpp
 * Titlebar {
 *     Horizontal {
 *         Icon { ":img/logo.svg" },
 *         Label { .text = AUI_REACT("{} - Editor"_format(*window->title())) },
 *     },
 * }
 * ```
 *
 * If you don't need contents in the titlebar area at all, use `DecorationDefault`: the OS draws the titlebar
 * entirely.
 *
 * ### What Titlebar is and how its size is determined
 *
 * [declarative::Titlebar] is not just a handle: it is a `Horizontal { window controls (start), contents, window controls (end) }`
 * container which stretches horizontally over the whole width of its parent. Its size is not set by the window or the
 * platform, it is regular AUI layout:
 *
 * - width: expanding, takes all the available width of the parent;
 * - height: the maximum of the contents' height and the window controls' height (native controls height from
 *   `AWindow::windowControlsInsets` or AUI-drawn buttons height). To make the titlebar taller, put a taller view or
 *   padding into it. In a Borderless window it is just the height of the contents.
 *
 * The rectangle occupied by [declarative::Titlebar] after layout is what the window treats as the caption area: it is used for
 * dragging and, on macOS and GTK, to vertically center the native window controls inside it.
 *
 * `AWindow::windowControlsInsets` is a property reporting the space occupied by native window controls (traffic lights on
 * macOS, `GtkWindowControls` on GTK); `AWindowControls` uses it to keep the contents out of their way.
 *
 * ## Lifecycle
 *
 * Only title and resizability can change at runtime:
 *
 * | What | API |
 * |---|---|
 * | title | `AWindow::title` property (`AWindow::setTitle`, `titleChanged`) |
 * | resizability | `AWindow::setResizable` / `AWindow::isResizable` |
 * | visibility | `AWindow::show` / `AWindow::hide`, reversible |
 * | closing | `AWindow::close`, **terminal** |
 *
 * `close()` works the same on all platforms: `closed` is emitted exactly once, the modal parent is unblocked, the native
 * window is destroyed. A closed window can't be shown again - create another one. Calling `show()` on a closed window logs
 * an error and does nothing. Closing by the close button calls `AWindow::onCloseButtonClicked`, which calls `close()`;
 * override it to refuse closing.
 *
 * ## Native handles
 *
 * `AWindow::nativeHandle` returns the underlying native window object, which can be passed to platform APIs AUI does not
 * wrap (DWM attributes, `NSWindow` tweaks, embedding into foreign toolkits, etc.). Its type is `AWindowNativePtr`
 * and depends on the platform:
 *
 * | Platform | `AWindowNativePtr` | What it points to |
 * |---|---|---|
 * | Windows | `HWND` | the window itself |
 * | macOS | `void*` | `NSWindow*` (`AUIView` is its content view) |
 * | Linux, GTK4 / libadwaita backend | `std::uintptr_t` | `GtkWindow*`; `GtkPopover*` for popups |
 * | Linux, X11 backend | `std::uintptr_t` | X11 `Window` (XID) |
 * | Android, iOS, web | `jobject` / `void*` | **not available**: always null, the surface is owned by the activity, view controller or canvas |
 *
 * The handle is null until the native window is created and after it is destroyed (see `AWindow::close`). It is owned by
 * AUI: don't destroy it, don't keep it past `AWindow::closed`. Use it from the UI thread only.
 *
 * ```cpp
 * #include <AUI/Platform/AWindow.h>
 *
 * void tweakNativeWindow(AWindow& window) {
 * #if AUI_PLATFORM_WIN
 *     HWND hwnd = window.nativeHandle();
 *     SetWindowTextW(hwnd, L"Hello from Win32");
 * #elif AUI_PLATFORM_APPLE
 *     // in an Objective-C++ (.mm) file:
 *     NSWindow* nsWindow = static_cast<NSWindow*>(window.nativeHandle());
 *     nsWindow.level = NSFloatingWindowLevel;
 * #elif AUI_PLATFORM_LINUX
 *     // see below
 * #endif
 * }
 * ```
 *
 * ### Linux
 *
 * Linux has two core platform APIs, chosen at runtime (see [APlatformAbstractionOptions]); on the application side
 * there is no compile-time way to tell which one is active, so the same `uintptr_t` means a different thing depending
 * on the backend. Decide which one you have yourself, i.e., by pinning
 * `APlatformAbstractionOptions::initializationOrder` to a single backend before creating windows:
 *
 * ```cpp
 * // X11 only
 * APlatformAbstractionOptions::set({ .initializationOrder = { APlatformAbstractionOptions::X11{} } });
 * ...
 * Window xid = static_cast<Window>(window.nativeHandle());
 * Display* display = XOpenDisplay(nullptr); // XIDs are valid on any connection to the same X server
 * ```
 *
 * ```cpp
 * // GTK4 / libadwaita only
 * APlatformAbstractionOptions::set({ .initializationOrder = { APlatformAbstractionOptions::Gtk4{} } });
 * ...
 * auto* gtkWindow = reinterpret_cast<GtkWindow*>(window.nativeHandle());
 * ```
 *
 * AUI loads GTK dynamically, so it does not require GTK headers to build your application; include them yourself if you
 * use the handle. To reach lower-level objects, go through GTK:
 *
 * - `GdkSurface*`: `gtk_native_get_surface(GTK_NATIVE(gtkWindow))`
 * - Wayland `wl_surface*`: `gdk_wayland_surface_get_wl_surface(gdkSurface)` (when `GDK_IS_WAYLAND_SURFACE`)
 * - X11 `Window` under GTK (i.e., GTK running on X11 or Xwayland): `gdk_x11_surface_get_xid(gdkSurface)` (when
 *   `GDK_IS_X11_SURFACE`)
 *
 * On a Wayland session without pinning the backend you will get the GTK flavor; the X11 backend works through Xwayland
 * there and its handle is an XID.
 *
 * ## Platform specifics
 *
 * | | Windows | macOS | Linux GTK/adw | Linux X11 | Android, iOS, web |
 * |---|---|---|---|---|---|
 * | `Default` buttons | window style bits; disabled close is greyed | style mask | `decoration-layout` of the header bar | Motif hints (window manager may ignore) | ignored |
 * | `Extended` window controls | drawn by AUI, hit-tested natively (snap layouts) | native traffic lights, aligned to [declarative::Titlebar] | native `GtkWindowControls` | drawn by AUI | none |
 * | `Extended` title ([declarative::WindowTitle]) | drawn by AUI | native, aligned to [declarative::Titlebar] vertically | native header bar title | drawn by AUI | drawn by AUI |
 * | `Borderless` shadow | DWM frame | system shadow | CSD shadow | **not available**, `shadow` is ignored | ignored |
 * | `parent` | owner window | child window | transient | transient | ignored |
 * | `modal` | parent is disabled | parent ignores input | `gtk_window_set_modal` | `_NET_WM_STATE_MODAL`, parent ignores input | ignored |
 *
 * Known gaps (not related to decorations): minimize/maximize are stubs on macOS, there is no `minimized` signal on all
 * platforms, `moveToCenter` is empty on Linux X11.
 */
class API_AUI_VIEWS AWindow: public ASurface
{
    friend class OpenGLRenderingContext;
    friend class IPlatformAbstraction;
    friend class CommonRenderingContext;
    friend class SoftwareRenderingContext;
    friend class AWindowManager;
    friend class AStubWindowManager;
    friend struct painter;
public:
    /**
     * @brief Window creation parameters.
     * @details
     * Pass it to the AWindow constructor to open a window with a parent, modality, custom decorations or fixed size;
     * for a plain window `AWindow(title, width, height)` is enough. See [AWindow] for how to open a window.
     *
     * Everything here except `title` and `resizable` is applied at window creation and can't be changed afterwards.
     * `title` and `resizable` are the initial values; use `AWindow::setTitle` and `AWindow::setResizable` to change them
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
            Buttons buttons = Buttons {};

            /**
             * @brief Whether the OS titlebar shows the application icon next to the title.
             * @details
             * Set it to false for dialog-like windows.
             *
             * @specificto{windows} Controls `WS_EX_DLGMODALFRAME`, the style that hides the icon. Ignored on the other
             * platforms: macOS and Linux window managers (including GTK/adw) have no such thing as an icon in the
             * titlebar of a window.
             */
            bool win32Icon = true;
        };

        /**
         * @brief Window contents extend into the titlebar.
         * @details
         * Native window controls are kept where the platform has them (traffic lights on macOS, window controls on
         * GTK) and reserve space via Titlebar; on the other platforms AUI draws them. Use Titlebar to fill the
         * titlebar.
         */
        struct DecorationExtended {
            Buttons buttons = Buttons {};

            /**
             * @brief Whether [declarative::Titlebar] draws the application icon (see `AWindow::windowIcon`) before
             *        its contents, the way a Windows titlebar does.
             * @details
             * Nothing is shown if the window has no icon.
             *
             * @specificto{windows} Ignored on the other platforms: macOS and Linux window managers (including GTK/adw)
             * have no such thing as an icon in the titlebar of a window.
             */
            bool win32Icon = true;
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

        /**
         * @brief Initial window title. Change it with `AWindow::setTitle`.
         */
        AString title = "My window";

        /**
         * @brief Initial window width.
         */
        AMetric width = 854_dp;

        /**
         * @brief Initial window height.
         */
        AMetric height = 500_dp;

        /**
         * @brief Parent window, if any. Makes the window a child (and optionally modal) of `Parent::window`.
         */
        AOptional<Parent> parent;

        /**
         * @brief Titlebar and border style. Can't be changed after creation.
         */
        Decoration decoration = DecorationDefault {};

        /**
         * @brief Whether the user can resize the window. Initial value; change it with `AWindow::setResizable`.
         */
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
     * @return true if the user input is blocked by `AWindow::blockUserInput` (i.e., a modal child window is open).
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
     * Read only. `Init::title` and `Init::resizable` are the initial values only; see `AWindow::title` and
     * `AWindow::isResizable` for actual ones.
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
     * @brief Window title: shown by the OS in the titlebar, taskbar, window switcher and so on.
     * @details
     * The window is the single source of truth for its title. To display the title inside the window (in a
     * [declarative::Titlebar] of an Extended or Borderless window), use [declarative::WindowTitle] or bind a label to
     * this property:
     *
     * ```cpp
     * Label { .text = AUI_REACT(*window->title()) }
     * ```
     */
    [[nodiscard]]
    auto title() const {
        return APropertyDef {
            this,
            &AWindow::mWindowTitle,
            &AWindow::setTitle,
            titleChanged,
        };
    }

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
     * @brief Space occupied by the window controls, in pixels.
     * @details
     * Non-zero only for `AWindow::Init::DecorationExtended`. Reported for both native (macOS, GTK) and AUI-drawn
     * (Windows, X11) window controls; the latter is known after the first layout pass.
     *
     * Android, iOS and web don't have window controls; the value is always zero.
     */
    AProperty<AWindowControlsInsets> windowControlsInsets;

    /**
     * @brief Parts of the window covered by the system UI, in pixels.
     * @details
     * Padding that guarantees the useful content is not overlapped by anything the application does not control:
     * the status bar, display cutout and navigation bar on Android and iOS; the window controls strip of
     * `AWindow::Init::DecorationExtended` on desktop. Put it into padding of the root view:
     *
     * ![](imgs/safe-zone.png)
     *
     * ```cpp
     * connect(window->safeAreaInsets, root, [root = root.get()](const ABoxFields& i) {
     *     root->setPadding(i);
     * });
     * ```
     *
     * The system UI itself is not part of the content area, so on desktop the value is zero if the window decoration
     * is the OS' one (`AWindow::Init::DecorationDefault`) or there is none (`AWindow::Init::DecorationBorderless`): the OS
     * (or you) already positions the content below the titlebar.
     *
     * For `AWindow::Init::DecorationExtended`, `top` is the height of `AWindow::windowControlsInsets`; [declarative::Titlebar]
     * lives inside that strip. Use `AWindow::windowControlsInsets` when you need to know which corners of the strip are
     * occupied, since the controls take a corner of it, not the whole strip.
     *
     * The software keyboard is not a part of the safe area.
     */
    AProperty<ABoxFields> safeAreaInsets;

    /**
     * @return true if the platform draws the window controls (traffic lights, etc.) of `AWindow::Init::DecorationExtended`
     * windows itself. Either way, they occupy `AWindow::windowControlsInsets`.
     */
    [[nodiscard]]
    bool hasNativeWindowControls() const;

    /**
     * @return true if the platform draws the title text of `AWindow::Init::DecorationExtended` window itself, over
     * [declarative::WindowTitle] placed into [declarative::Titlebar]. True on macOS and Linux GTK/adw.
     */
    [[nodiscard]]
    bool hasNativeWindowTitle() const;

    /**
     * @brief Whether the specified point (in window coordinates) belongs to the titlebar, i.e., can be used to drag the
     *        window.
     * @details
     * A point is a caption if the deepest view there is inside the titlebar and is neither an [AButton] nor has
     * `.override-title-dragging` ASS class.
     */
    [[nodiscard]]
    bool isCaptionAt(const glm::ivec2& pos);

    /**
     * @brief Registers the titlebar of this window. Called by ATitlebar (implementation of [declarative::Titlebar]).
     */
    void setTitlebar(ATitlebar* titlebar) noexcept { mTitlebar = titlebar; }

    [[nodiscard]]
    ATitlebar* titlebar() const noexcept { return mTitlebar; }

    /**
     * @return icon of the window: the one set by `AWindow::setIcon`, or the `MAINICON` resource of the executable. Used
     * to draw the icon in [declarative::Titlebar].
     * @specificto{windows} Null on the other platforms.
     */
    [[nodiscard]]
    const _<IDrawable>& windowIcon() const noexcept { return mWindowIcon; }

    /**
     * @brief `AWindow::windowIcon` has changed.
     */
    emits<> windowIconChanged;

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

    [[nodiscard]]
    bool isFrameScheduled() const noexcept override {
#if AUI_PLATFORM_WIN
        return !mRedrawFlag;   // on Windows the flag means "a redraw may be requested", see flagRedraw
#else
        return mRedrawFlag;
#endif
    }

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
     * Use `AWindow::hide` to hide the window temporarily.
     */
    void close();

    /**
     * @brief Hides the window without closing it.
     * @details
     * A hidden window is still alive: it is counted as an open window, so it keeps the application running
     * (see `AApplication::quitOnLastWindowClosed`). Use `AWindow::close` to destroy it.
     *
     * A hidden window is not visible anywhere, including the taskbar.
     */
    void hide();

    /**
     * @brief Shows the window, restores it if minimized and brings it to foreground.
     * @param activationToken platform-specific focus stealing permission token (see `AActivation::activationToken`).
     * @details
     * Typical usage is handling `AApplication::activated`:
     * ```cpp
     * connect(AApplication::inst().activated, [window](const AActivation& a) { window->activate(a.activationToken); });
     * ```
     */
    void activate(const AString& activationToken = {});

    [[nodiscard]]
    bool isClosed() const noexcept;

    [[nodiscard]]
    AWindowNativePtr nativeHandle() const noexcept {
        return mHandle;
    }

    glm::ivec2 getWindowPosition() const;

    /**
     * @brief The smallest size this window can be resized to.
     * @details
     * In contrast to `AView::getMinSize`, which is the explicitly styled minimum, this one is measured from the
     * window's contents: a window is never smaller than the space its contents need at their narrowest, so its
     * layout is never squeezed into nothing. The styled minimum is respected as well.
     *
     * The minimum height depends on the current width of the window (wrapped text needs less height in a wider
     * window). Note that this differs from GTK and Qt, which use the height needed at the minimum width (a static
     * value, the tallest the contents can be). As the minimum height changes along with the width, it is
     * re-announced to the window manager on every layout; on win32 it may lag one resize step behind.
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
     * @return created window, `AWindow::show()` is not called
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
    /**
     * @brief Emitted at the end of every frame, after the contents of the window are drawn.
     * @details
     * Whatever a slot draws (with `getRenderingContext()->renderer()`) appears on top of everything else. It is the
     * place for things which are not part of the UI, i.e., the remote control indication of `aui::dev_extras`. A slot
     * that animates should call AWindow::flagRedraw by itself.
     */
    emits<> postRender;

    emits<> closed;
    emits<> shown;

    /**
     * @brief Window title has changed.
     */
    emits<AString> titleChanged;

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
    _<IDrawable> mWindowIcon;

    /**
     * @brief Resizability reported when there's no native window to ask (i.e., under UI tests).
     */
    bool mResizableCache = true;

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
    AOptional<ARect<int>> mLastTitlebarRect;
    AOptional<ARect<int>> mLastWindowTitleRect;
    void applyResizeEdgeCursor(glm::vec2 pos);
#endif
#if AUI_PLATFORM_MACOS
    void applyTitlebarRect();
    void updateWindowControlsInsets();
#endif
    /**
     * @brief Publishes `AWindow::windowControlsInsets` of AUI-drawn window controls. Called after layout.
     */
    void updateDrawnWindowControlsInsets();
    void updateSafeAreaInsetsFromControls();
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
