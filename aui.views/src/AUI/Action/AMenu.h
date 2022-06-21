/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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

