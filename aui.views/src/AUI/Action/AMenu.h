// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
struct AMenuItem;

using AMenuModel = AVector<AMenuItem>;

class AMenu {
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

