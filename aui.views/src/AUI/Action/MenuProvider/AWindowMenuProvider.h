/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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


#include "IMenuProvider.h"
#include <AUI/Platform/AWindow.h>

class AMenuContainer;

class AWindowMenuProvider: public IMenuProvider {
private:
    ABaseWindow* mWindow;

    _weak<AMenuContainer> mMenuContainer;

public:
    void createMenu(const AVector<AMenuItem>& vector) override;

    bool isOpen() override;

    void closeMenu() override;
};


