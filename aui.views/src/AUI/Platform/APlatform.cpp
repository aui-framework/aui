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
// Created by nelonn on 11/14/25.
//

#include "APlatform.h"

#if AUI_PLATFORM_LINUX
#include <AUI/Platform/linux/IPlatformAbstraction.h>
#elif AUI_PLATFORM_MACOS
#include <AUI/Platform/macos/Platform.h>
#elif AUI_PLATFORM_WINDOWS
#include <AUI/Platform/win32/Platform.h>
#elif AUI_PLATFORM_IOS
#include <AUI/Platform/ios/Platform.h>
#elif AUI_PLATFORM_ANDROID
#include <AUI/Platform/android/Platform.h>
#elifAUI_PLATFORM_EMSCRIPTEN
#include <AUI/Platform/emscripten/Platform.h>
#endif

std::unique_ptr<APlatform>& APlatform::currentImpl() {
    static std::unique_ptr<APlatform> impl{};
    return impl;
}

void APlatform::init(std::unique_ptr<APlatform>&& platform) {
    currentImpl() = std::move(platform);
}

APlatform& APlatform::current() {
    auto& impl = currentImpl();
    if (!impl) {
#if AUI_PLATFORM_LINUX
        impl = IPlatformAbstraction::create();
#elif AUI_PLATFORM_MACOS
        impl = std::make_unique<aui::PlatformMacOS>();
#elif AUI_PLATFORM_WINDOWS
        impl = std::make_unique<aui::PlatformWin32>();
#elif AUI_PLATFORM_IOS
        impl = std::make_unique<aui::PlatformIOS>();
#elif AUI_PLATFORM_ANDROID
        impl = std::make_unique<aui::PlatformAndroid>();
#elif AUI_PLATFORM_EMSCRIPTEN
        impl = std::make_unique<aui::PlatformWeb>();
#else
        throw AException("You must initialize APlatform");
#endif
    }
    return *impl;
}
