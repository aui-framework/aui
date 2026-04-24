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

#if AUI_PLATFORM_MACOS
#include "AUI/Platform/macos/MacOSMenuProvider.h"
#elif AUI_PLATFORM_WIN
#include "AUI/Platform/win32/Win32MenuProvider.h"
#endif

namespace {
bool gUseSystemMenus = false;
_<IMenuProvider> gOverrideProvider;
_<IMenuProvider> gCachedProvider;

void resetCache() { gCachedProvider.reset(); }
}

void AMenu::show(const AMenuModel& model) {
    provider()->createMenu(model);
}

_<IMenuProvider>& AMenu::provider() {
    if (gCachedProvider) {
        return gCachedProvider;
    }

    if (gOverrideProvider) {
        gCachedProvider = gOverrideProvider;
        return gCachedProvider;
    }

    if (!dynamic_cast<AWindow*>(AWindow::current())) {
        gCachedProvider = _new<AEmbedMenuProvider>();
        return gCachedProvider;
    }

    if (gUseSystemMenus) {
#if AUI_PLATFORM_MACOS
        gCachedProvider = _new<MacOSMenuProvider>();
        return gCachedProvider;
#elif AUI_PLATFORM_WIN
        gCachedProvider = _new<Win32MenuProvider>();
        return gCachedProvider;
#endif
        // No system-menu implementation on this platform — fall through to AUI-drawn.
    }

    gCachedProvider = _new<AWindowMenuProvider>();
    return gCachedProvider;
}

void AMenu::close() {
    provider()->closeMenu();
}

bool AMenu::isOpen() {
    return provider()->isOpen();
}

void AMenu::setUseSystemMenus(bool enabled) {
    if (gUseSystemMenus == enabled) {
        return;
    }
    gUseSystemMenus = enabled;
    resetCache();
}

void AMenu::setGlobalProvider(_<IMenuProvider> provider) {
    gOverrideProvider = std::move(provider);
    resetCache();
}
