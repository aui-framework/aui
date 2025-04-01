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
