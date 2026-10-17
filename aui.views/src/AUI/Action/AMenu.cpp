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
bool gUseSystemMenus = true;
std::unique_ptr<IMenuProvider> gOverrideProvider;
std::unique_ptr<IMenuProvider> gCachedProvider;

void resetCache() { gCachedProvider.reset(); }
}

void AMenu::show(const AMenuModel& model) {
    provider().createMenu(model);
}

IMenuProvider& AMenu::provider() {
    if (gOverrideProvider) {
        return *gOverrideProvider;
    }

    if (gCachedProvider) {
        return *gCachedProvider;
    }

    if (!dynamic_cast<AWindow*>(ASurface::current())) {
        gCachedProvider = std::make_unique<AEmbedMenuProvider>();
        return *gCachedProvider;
    }

    if (gUseSystemMenus) {
#if AUI_PLATFORM_MACOS
        gCachedProvider = std::make_unique<MacOSMenuProvider>();
        return *gCachedProvider;
#elif AUI_PLATFORM_WIN
        gCachedProvider = std::make_unique<Win32MenuProvider>();
        return *gCachedProvider;
#endif
        // No system-menu implementation on this platform — fall through to AUI-drawn.
    }

    gCachedProvider = std::make_unique<AWindowMenuProvider>();
    return *gCachedProvider;
}

void AMenu::close() {
    provider().closeMenu();
}

bool AMenu::isOpen() {
    return provider().isOpen();
}

bool AMenu::usingSystemMenus() noexcept { return gUseSystemMenus; }

void AMenu::setUseSystemMenus(bool enabled) {
    if (gUseSystemMenus == enabled) {
        return;
    }
    if (gCachedProvider && gCachedProvider->isOpen()) {
        gCachedProvider->closeMenu();
    }
    gUseSystemMenus = enabled;
    resetCache();
}

void AMenu::setGlobalProvider(std::unique_ptr<IMenuProvider> provider) {
    if (gOverrideProvider && gOverrideProvider->isOpen()) {
        gOverrideProvider->closeMenu();
    }
    if (gCachedProvider && gCachedProvider->isOpen()) {
        gCachedProvider->closeMenu();
    }
    gOverrideProvider = std::move(provider);
    resetCache();
}
