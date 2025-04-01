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

