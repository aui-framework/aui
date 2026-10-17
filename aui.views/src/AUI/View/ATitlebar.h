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

#include <AUI/View/AViewContainer.h>
#include <AUI/View/ALabel.h>

class AWindow;
class AButton;
class ATitlebar;

/**
 * @brief Native window controls (close, minimize, maximize) placed at one side of ATitlebar.
 * @ingroup views
 * @details
 * Where the platform draws the window controls itself (macOS traffic lights, GTK window controls), AWindowControls is
 * a blank space of the size of the native controls (AWindow::windowControlsInsets). Elsewhere (Windows, X11), these are
 * AUI buttons following AWindow::Init::Buttons.
 *
 * AWindowControls is a part of ATitlebar and is not intended to be used standalone.
 */
class API_AUI_VIEWS AWindowControls : public AViewContainer {
public:
    enum class Side {
        START,
        END,
    };

    explicit AWindowControls(Side side);

    void onViewGraphSubtreeChanged() override;

    enum class Button {
        MINIMIZE,
        MAXIMIZE,
        CLOSE,
    };

    /**
     * @return button of the AUI-drawn window controls at the specified point, relative to the window. Used for
     * hit-testing in the non-client area.
     */
    [[nodiscard]] AOptional<Button> buttonAt(glm::ivec2 positionInWindow) const;

    /**
     * @brief Toggles hover look of AUI-drawn window button. For platforms which handle pointer over window buttons
     *        themselves (non-client area).
     */
    void setButtonNativeHover(AOptional<Button> button);

private:
    Side mSide;
    AWindow* mWindow = nullptr;
    _<AButton> mMinimize;
    _<AButton> mMaximize;
    _<AButton> mClose;

    void build(AWindow& window);
    void updateMaximizeIcon();
};

/**
 * Implementation detail of [declarative::WindowTitle]; create it with `WindowTitle {}`. The class is exposed only to
 * be used in ASS selectors (`t<AWindowTitle>()`).
 */
class API_AUI_VIEWS AWindowTitle : public ALabel {
public:
    AWindowTitle();
    ~AWindowTitle() override;
    void onViewGraphSubtreeChanged() override;
    void render(ARenderContext context) override;

    /**
     * @return true if the title text is drawn by the OS over this view (see AWindow::hasNativeWindowTitle), i.e., this
     * view only reserves space for it.
     */
    [[nodiscard]]
    bool isNative() const noexcept;

private:
    AWindow* mWindow = nullptr;
    ATitlebar* mTitlebar = nullptr;
    friend class ATitlebar;
};

namespace declarative {
struct Titlebar;

/**
 * @brief Displays the title of the window it is placed in, the way the OS displays it in its own titlebar.
 * @ingroup views
 * @details
 * The text is bound to AWindow::title of the window WindowTitle is placed in, so AWindow::setTitle updates both the
 * OS-visible title (taskbar, window switcher) and the title drawn inside the window. There is nothing to keep in sync.
 *
 * Where the OS can draw the title of a DecorationExtended window itself (see AWindow::hasNativeWindowTitle), it does:
 * WindowTitle placed into [declarative::Titlebar] is a blank space, and the native title is drawn over it with the
 * system font, color and inactive window look. The native title is shown only while the window's titlebar contains a
 * WindowTitle.
 *
 * - macOS: the title is drawn by AppKit, centered over the whole window horizontally (the horizontal position of
 *   WindowTitle doesn't matter) and centered vertically in [declarative::Titlebar].
 * - Linux GTK/adw: the title is a native header bar title. Like in a regular header bar, it is centered over the
 *   window while it fits into WindowTitle, and is shifted into WindowTitle otherwise.
 *
 * Elsewhere (Windows, X11, Extended windows with AUI-drawn controls, Borderless windows) there is no native title to
 * reuse, and WindowTitle is a label drawn by AUI: left-aligned on Windows, centered on Linux. Style it with
 * `t<AWindowTitle>()` ASS selector. ASS text properties have no effect on the native title.
 *
 * WindowTitle is expanding horizontally.
 *
 * `Titlebar {}` without contents contains WindowTitle. Put WindowTitle explicitly to combine it with other views:
 *
 * ```cpp
 * Titlebar {
 *   Horizontal {
 *     Icon { ":img/logo.svg" },
 *     WindowTitle {},
 *     Button { Label { "Settings" } },
 *   },
 * }
 * ```
 */
struct WindowTitle {
    _<AWindowTitle> operator()() { return _new<AWindowTitle>(); }
};
}   // namespace declarative

/**
 * Implementation detail of [declarative::Titlebar]; create titlebars with `Titlebar { ... }`. The class is exposed
 * only to be used in ASS selectors (`t<ATitlebar>()`) and for hit-testing in AWindow.
 */
class API_AUI_VIEWS ATitlebar : public AViewContainer {
public:
    ~ATitlebar() override;

    void onViewGraphSubtreeChanged() override;

    /**
     * @return AUI-drawn window button at the specified point (in window coordinates), if any.
     */
    [[nodiscard]] AOptional<AWindowControls::Button> windowButtonAt(glm::ivec2 positionInWindow) const;

    /**
     * @brief Reflects the pointer hovering a window button handled by the OS (non-client area).
     */
    void setWindowButtonNativeHover(AOptional<AWindowControls::Button> button);

    /**
     * @return WindowTitle inside this titlebar, if any.
     */
    [[nodiscard]]
    AWindowTitle* windowTitle() const noexcept { return mWindowTitle; }

private:
    explicit ATitlebar(_<AView> contents);

    AWindow* mWindow = nullptr;
    AWindowTitle* mWindowTitle = nullptr;
    _<AWindowControls> mStart;
    _<AWindowControls> mEnd;
    friend class AWindow;
    friend class AWindowTitle;
    friend struct declarative::Titlebar;
};

namespace declarative {
/**
 * @brief Titlebar of the window which contents extend into the titlebar area (AWindow::Init::DecorationExtended).
 * @ingroup views
 * @details
 * Titlebar is the AUI's equivalent of GtkWindowHandle + AdwHeaderBar. It lays out as
 * `Horizontal { window controls (start), contents (expanding), window controls (end) }`. Its height is the maximum of
 * the contents' height and the window controls' height, so making the titlebar higher is as simple as putting a taller
 * view in it.
 *
 * Titlebar is always a window drag handle: dragging any point under it, except buttons ([AButton]) and views with
 * `.override-title-dragging` ASS class, moves the window; double click maximizes/restores it.
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
 * @specificto{android,ios,web} Titlebar just displays its contents; it does not drag the window and does not have
 * window controls.
 */
struct Titlebar {
    /**
     * @brief Contents of the titlebar. Defaults to [declarative::WindowTitle], so `Titlebar {}` looks like the
     * titlebar the OS would draw.
     */
    _<AView> content = WindowTitle {}();

    _<AView> operator()() {
        return aui::ptr::manage_shared(new ATitlebar(std::move(content)));
    }
};
}   // namespace declarative
