// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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


#include <AUI/Action/MenuProvider/AWindowMenuProvider.h>
#include <AUI/Action/MenuProvider/AEmbedMenuProvider.h>
#include "AMenu.h"

void AMenu::show(const AMenuModel& model) {
    provider()->createMenu(model);
}

_<IMenuProvider>& AMenu::provider() {
    static _<IMenuProvider> provider = nullptr;
    if (provider == nullptr) {
        if (dynamic_cast<AWindow*>(AWindow::current())) {
            provider = _new<AWindowMenuProvider>();
        } else {
            provider = _new<AEmbedMenuProvider>();
        }
    }

    return provider;
}

void AMenu::close() {
    provider()->closeMenu();
}

bool AMenu::isOpen() {
    return provider()->isOpen();
}
