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


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "AShortcut.h"

class IMenuProvider;
class IDrawable;
struct AMenuItem;

using AMenuModel = AVector<AMenuItem>;

class API_AUI_VIEWS AMenu {
private:
    static _<IMenuProvider>& provider();
public:
    static void show(const AMenuModel& model);
    static void close();
    static bool isOpen();

    /**
     * @brief Switches context menu rendering between the platform-native and AUI-drawn providers.
     * @details
     * By default (opt out) AUI uses native `NSMenu` on macOS and `HMENU` (`TrackPopupMenuEx`) on
     * Windows — giving native appearance, VoiceOver/Narrator integration, and system-level animations.
     * Calling `setUseSystemMenus(false)` opts out and makes AUI draw its own menus for cross-platform
     * theme consistency. On Linux/X11 there is no system-wide native context menu API; this flag is
     * a no-op and the AUI-drawn provider is always used.
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
    static void setGlobalProvider(_<IMenuProvider> provider);

    enum Type {
        SINGLE,
        SUBLIST,
        SEPARATOR
    };
};


struct AMenuItem {
    AMenu::Type type = AMenu::SINGLE;
    AString name;
    AShortcut shortcut;
    std::function<void()> onAction;
    AVector<AMenuItem> subItems;
    bool enabled = true;
    _<IDrawable> icon;
};
