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


#pragma once

#include <AUI/Action/AMenu.h>

class IMenuProvider {
public:
    virtual void createMenu(const AVector<AMenuItem>& vector) = 0;
    virtual void closeMenu() = 0;
    virtual bool isOpen() = 0;
};