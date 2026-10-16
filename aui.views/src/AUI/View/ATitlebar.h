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

class AWindow;
class AButton;

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
 * @brief Titlebar of the window which contents extend into the titlebar area (AWindow::Init::DecorationExtended).
 * @ingroup views
 * @details
 * ATitlebar is the AUI's equivalent of GtkWindowHandle + AdwHeaderBar. It lays out as
 * `Horizontal { AWindowControls(START), contents (expanding), AWindowControls(END) }`. Its height is the maximum of
 * the contents' height and the window controls' height, so making the titlebar higher is as simple as putting a taller
 * view in it.
 *
 * ATitlebar is always a window drag handle: dragging any point under it, except buttons ([AButton]) and views with
 * `.override-title-dragging` ASS class, moves the window; double click maximizes/restores it.
 *
 * ```cpp
 * setContents(Stacked {
 *     content,
 *     Vertical {
 *         _new<ATitlebar>(Horizontal { Label { "My app" } }),
 *         SpacerExpanding {},
 *     },
 * });
 * ```
 *
 * @specificto{android,ios,web} ATitlebar just displays its contents; it does not drag the window and does not have
 * window controls.
 */
class API_AUI_VIEWS ATitlebar : public AViewContainer {
public:
    explicit ATitlebar(_<AView> contents = nullptr);
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

private:
    AWindow* mWindow = nullptr;
    _<AWindowControls> mStart;
    _<AWindowControls> mEnd;
    friend class AWindow;
};
