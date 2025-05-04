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

#include "AUI/Platform/linux/gtk/PlatformAbstractionGtk.h"
#include "AUI/Platform/linux/x11/PlatformAbstractionX11.h"
#include "IPlatformAbstraction.h"

static constexpr auto LOG_TAG = "IPlatformAbstraction";

IPlatformAbstraction::IPlatformAbstraction() {
    ALogger::info(LOG_TAG) << "Desktop Environment: " << std::getenv("XDG_CURRENT_DESKTOP");
}

IPlatformAbstraction& IPlatformAbstraction::current() {
    static IPlatformAbstraction& value = []() -> IPlatformAbstraction& {
        std::string_view auiPa = "gtk";
        if (auto value = std::getenv("AUI_PA")) {
            auiPa = value;
        }
        if (auiPa == "gtk") {
            static PlatformAbstractionGtk result;
            return result;
        }

        static PlatformAbstractionX11 x11;
        return x11;
    }();
    return value;
}

void IPlatformAbstraction::setCurrentWindow(AWindowBase* window) {
    AWindow::currentWindowStorage() = window;
}

float IPlatformAbstraction::windowGetDpiRatio(AWindow& window) {
    return platformGetDpiRatio();
}
