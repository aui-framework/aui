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

IPlatformAbstraction::IPlatformAbstraction() {
    auto e = std::getenv("XDG_CURRENT_DESKTOP");
    ALogger::info(LOG_TAG) << "Desktop Environment: " << (e ? e : "none");
}

IPlatformAbstraction& IPlatformAbstraction::current() {
    static auto value = []() -> _unique<IPlatformAbstraction> {
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
    }();
    return *value;
}

void IPlatformAbstraction::setCurrentWindow(ASurface* window) {
    AWindow::currentWindowStorage() = window;
}

float IPlatformAbstraction::windowGetDpiRatio(AWindow& window) {
    return platformGetDpiRatio();
}
