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
     * @brief Switches context menu rendering to the platform-native provider.
     * @details
     * By default AUI draws its own menus for cross-platform theme consistency. Calling
     * `setUseSystemMenus(true)` opts in to native `NSMenu` on macOS and `HMENU`
     * (`TrackPopupMenuEx`) on Windows — giving native appearance, VoiceOver/Narrator
     * integration, and system-level animations. On Linux/X11 there is no system-wide
     * native context menu API; this flag is a no-op and the AUI-drawn provider is kept.
     */
    static void setUseSystemMenus(bool enabled = true);

    /**
     * @brief Install a custom provider (overrides both the default and the system setting).
     * @details
     * Useful for testing, embedded renderers, or apps that want a fully custom menu look
     * and feel. Pass `nullptr` to reset to the default factory behaviour.
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

    /**
     * @brief Optional icon shown to the left of the menu item label.
     * @details
     * Honoured by native providers (NSMenu / HMENU) when `AMenu::setUseSystemMenus(true)`
     * and by the AUI-drawn provider. On macOS the drawable is rasterised to 16pt, on
     * Windows to 16×16 physical pixels.
     */
    _<IDrawable> icon;
};

