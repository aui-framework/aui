// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 5/13/2021.
//


#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "AShortcut.h"

class IMenuProvider;
struct MenuItem;

using MenuModel = AVector<MenuItem>;

class AMenu {
private:
    static _<IMenuProvider>& provider();
public:
    static void show(const MenuModel& model);
    static void close();
    static bool isOpen();

    enum Type {
        SINGLE,
        SUBLIST,
        SEPARATOR
    };
};


struct MenuItem {
    AMenu::Type type;
    AString name;
    AShortcut shortcut;
    std::function<void()> onAction;
    AVector<MenuItem> subItems;
    bool enabled;


    MenuItem(const AString& name, const std::function<void()>& onAction = {}, const AShortcut& shortcut = {}, bool enabled = true):
        type(AMenu::SINGLE),
        name(name),
        onAction(onAction),
        shortcut(shortcut),
        enabled(enabled) {

    }
    MenuItem(const AString& name, const AVector<MenuItem>& subItems, bool enabled = true):
        type(AMenu::SUBLIST),
        name(name),
        subItems(subItems),
        enabled(enabled) {

    }
    MenuItem(AMenu::Type t): type(t) {

    }
};

