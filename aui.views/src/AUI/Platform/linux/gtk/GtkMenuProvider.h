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
#include <AUI/Action/MenuProvider/AWindowMenuProvider.h>

/**
 * @brief Native GTK4/libadwaita context-menu provider.
 * @details
 * Builds a GMenu tree from an AMenuModel and displays it as GtkPopoverMenu at the mouse position. Gives the native
 * appearance of the current GTK/libadwaita theme.
 *
 * Unlike the Win32 and macOS providers, GtkPopoverMenu is not modal: createMenu returns immediately and the item
 * callback is invoked later from the main loop.
 */
class GtkMenuProvider : public IMenuProvider {
public:
    GtkMenuProvider();
    ~GtkMenuProvider() override;

    void createMenu(const AVector<AMenuItem>& vector) override;
    void closeMenu() override;
    bool isOpen() override;

    /**
     * @return true if the current platform abstraction is GTK4/libadwaita.
     */
    [[nodiscard]] static bool isSupported();

private:
    void* mPopover = nullptr;   // GtkWidget*; gio headers can't be included here because of the `signals` macro.
    AWindowMenuProvider mFallback;   // used when the current surface has no GTK widget to attach the popover to.

    static void onClosed(void* popover, GtkMenuProvider* self);
    void detach();
};
