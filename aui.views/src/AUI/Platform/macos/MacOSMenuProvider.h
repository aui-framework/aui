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
 * @brief Native macOS context-menu provider.
 * @details
 * Builds an NSMenu tree from an AMenuModel and displays it via
 * `[NSMenu popUpMenuPositioningItem:atLocation:inView:]`. Gives AppKit's
 * native appearance, key-equivalents, VoiceOver integration, and system
 * translucency.
 */
class API_AUI_VIEWS MacOSMenuProvider : public IMenuProvider {
public:
    MacOSMenuProvider();
    ~MacOSMenuProvider() override;

    void createMenu(const AVector<AMenuItem>& vector) override;
    void closeMenu() override;
    bool isOpen() override;

private:
    // Opaque pointers to Objective-C objects (NSMenu*, NSMutableArray*). The headers
    // stay C++-only so non-.mm translation units can include this.
    void* mCurrentMenu = nullptr;
    void* mTrampolines = nullptr;
    bool mOpen = false;
};
