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

//
// Created by alex2 on 5/13/2021.
//


#pragma once


#include <memory>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "AShortcut.h"

class IMenuProvider;
class IDrawable;
struct AMenuItem;

using AMenuModel = AVector<AMenuItem>;

/**
 * @brief Popup (context) menu.
 * @ingroup views
 * @details
 * A menu is described with a model - a vector of `AMenuItem` structures with a name, an optional shortcut hint,
 * and an `onAction` handler.
 *
 * The most common way to show a menu is to override `AView::composeContextMenu`; it is called when the user right
 * clicks the view (or long presses it on touchscreens). Return an empty model to show nothing.
 *
 * ```cpp
 * class MyView : public AView {
 * protected:
 *     AMenuModel composeContextMenu() override {
 *         return {
 *             { .name = "Copy", .shortcut = AInput::CMD_CTRL + AInput::C, .onAction = [this] { copy(); } },
 *             { .type = AMenu::SEPARATOR },
 *             { .name = "Disabled", .enabled = false },
 *         };
 *     }
 * };
 * ```
 *
 * To show a menu on your own, call `AMenu::show(model)`. Menus are closed with `AMenu::close()`; `AMenu::isOpen()`
 * tells whether there's an open menu.
 *
 * ## Appearance
 *
 * Menus are rendered by the platform, so they look native.
 *
 * | Platform                          | Implementation                | Example                                |
 * |-----------------------------------|-------------------------------|----------------------------------------|
 * | Windows                           | `HMENU` (`TrackPopupMenuEx`)  | ![](imgs/amenu_win.png)                |
 * | macOS                             | `NSMenu`                      | ![](imgs/amenu_macos.png)              |
 * | Linux (GTK4, GNOME)               | `GtkPopoverMenu`              | ![](imgs/amenu_gnome.png)              |
 * | `AMenu::setUseSystemMenus(false)` | AUI-drawn menu                | ![](imgs/amenu_fallback.png)           |
 *
 * !!! note
 *
 *     The final look of the menu depends on the OS version and, on Linux, on the desktop environment (and its theme).
 *     The screenshots above are examples and may differ from what your users see.
 *
 * !!! note
 *
 *     `shortcut` is a text hint only; AUI does not register it as a global hotkey. Handle the keys yourself, see
 *     [ui-input-events].
 */
class API_AUI_VIEWS AMenu {
private:
    static IMenuProvider& provider();
public:
    /**
     * @brief Shows the menu described by `model`.
     */
    static void show(const AMenuModel& model);

    /**
     * @brief Closes the currently open menu, if any.
     */
    static void close();

    /**
     * @brief Returns true if a menu is currently open.
     */
    static bool isOpen();

    /**
     * @brief Switches context menu rendering between the platform-native and AUI-drawn providers.
     * @details
     * By default (opt out) AUI uses native `NSMenu` on macOS, `HMENU` (`TrackPopupMenuEx`) on
     * Windows and `GtkPopoverMenu` on Linux with GTK4/libadwaita — giving native appearance,
     * screen reader integration, and system-level animations.
     * Calling `setUseSystemMenus(false)` opts out and makes AUI draw its own menus for cross-platform
     * theme consistency. On Linux without GTK4 (i.e., plain X11) there is no native context menu API; this flag
     * is a no-op and the AUI-drawn provider is always used.
     * Call from the UI thread; the cached provider is swapped without synchronization.
     */
    static void setUseSystemMenus(bool enabled = true);

    /**
     * @brief Returns the value last passed to setUseSystemMenus (`true` by default).
     */
    [[nodiscard]] static bool usingSystemMenus() noexcept;

    /**
     * @brief Install a custom provider (overrides both the default and the system setting).
     * @details
     * Useful for testing, embedded renderers, or apps that want a fully custom menu look
     * and feel. Pass `nullptr` to reset to the default factory behaviour.
     * Call from the UI thread; the cached provider is swapped without synchronization.
     */
    static void setGlobalProvider(std::unique_ptr<IMenuProvider> provider);

    /**
     * @brief Menu item type.
     */
    enum Type {
        /// Regular clickable item.
        SINGLE,

        /// Item that opens a nested menu from `AMenuItem::subItems`.
        SUBLIST,

        /// Horizontal divider.
        SEPARATOR
    };
};


/**
 * @brief An item of [AMenuModel].
 * @details
 * Fields not applicable to the item's `type` are ignored. Whether `shortcut` and `icon` are displayed depends on the
 * platform; see [AMenu].
 */
struct AMenuItem {
    AMenu::Type type = AMenu::SINGLE;
    AString name;
    AShortcut shortcut;
    std::function<void()> onAction;
    AVector<AMenuItem> subItems;
    bool enabled = true;
    _<IDrawable> icon;
};
