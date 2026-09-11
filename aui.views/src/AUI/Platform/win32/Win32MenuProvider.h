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

#include <AUI/Action/MenuProvider/IMenuProvider.h>

/**
 * @brief Native Win32 context-menu provider.
 * @details
 * Builds an HMENU tree from an AMenuModel and displays it via `TrackPopupMenuEx`.
 * Produces Windows 11 fluent appearance, dark-mode awareness, system accent colors,
 * keyboard navigation, and Narrator integration for free.
 */
class API_AUI_VIEWS Win32MenuProvider : public IMenuProvider {
public:
    Win32MenuProvider();
    ~Win32MenuProvider() override;

    void createMenu(const AVector<AMenuItem>& vector) override;
    void closeMenu() override;
    bool isOpen() override;

private:
    bool mOpen = false;
};
