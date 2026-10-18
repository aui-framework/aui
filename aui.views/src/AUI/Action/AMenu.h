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
 * !!! note
 *
 *     `shortcut` is a text hint only; AUI does not register it as a global hotkey. Handle the keys yourself, see
 *     [ui-input-events].
 */
class API_AUI_VIEWS AMenu {
private:
    static _<IMenuProvider>& provider();
public:
    static void show(const AMenuModel& model);
    static void close();
    static bool isOpen();

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
};

