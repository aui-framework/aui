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

#include "IPlatformAbstraction.h"
#include "APlatformAbstractionOptions.h"

static constexpr auto LOG_TAG = "IPlatformAbstraction";

std::unique_ptr<IPlatformAbstraction> IPlatformAbstraction::create() {
    for (const auto& option : APlatformAbstractionOptions::get().initializationOrder) {
        auto name = option.target_type().name();
        try {
            auto value = option();
            value->init();
            ALogger::info(LOG_TAG) << "Using \"" << name << "\"";
            return value;
        } catch (const AException& e) {
            ALogger::info(LOG_TAG) << "\"" << name << "\" failed to initialize: " << e.getMessage();
        }
    }
    throw AException("can't find a suitable platform abstraction (maybe check AUI_PA environment variable?)");
}

IPlatformAbstraction& IPlatformAbstraction::current() {
    APlatform& platform = APlatform::current();
    auto* native_platform = dynamic_cast<IPlatformAbstraction*>(&platform);
    if (!native_platform) {
        throw AException("Since the current platform is not AUI's native implementation, some system methods are unavailable.");
    }
    return *native_platform;
}

IPlatformAbstraction* IPlatformAbstraction::currentSafe() {
    APlatform& platform = APlatform::current();
    return dynamic_cast<IPlatformAbstraction*>(&platform);
}

IPlatformAbstraction::IPlatformAbstraction() {
    auto e = std::getenv("XDG_CURRENT_DESKTOP");
    ALogger::info(LOG_TAG) << "Desktop Environment: " << (e ? e : "none");
}

void IPlatformAbstraction::setCurrentWindow(ASurface* window) {
    AWindow::currentWindowStorage() = window;
}

float IPlatformAbstraction::windowGetDpiRatio(AWindow& window) {
    return 1.0f;
}
