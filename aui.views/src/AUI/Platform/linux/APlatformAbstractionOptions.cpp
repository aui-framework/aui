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
#include "AUI/Platform/linux/adw/PlatformAbstractionAdw.h"
#include "APlatformAbstractionOptions.h"

using std::operator""sv;

AVector<APlatformAbstractionOptions::InitializationVariant> APlatformAbstractionOptions::defaultInitializationOrder() {
    /// [APlatformAbstractionOptions::defaultInitializationOrder]
    if (auto auiPa = std::getenv("AUI_PA")) {
        if (auiPa == "adw1"sv) {
            return { Adwaita1 {} };
        }
        if (auiPa == "gtk4"sv) {
            return { Gtk4 {} };
        }
        if (auiPa == "x11"sv) {
            return { X11 {} };
        }
        ALogger::err("APlatformAbstractionOptions") << "Unknown AUI_PA \"" << auiPa << "\", ignoring";
    }
    AVector<APlatformAbstractionOptions::InitializationVariant> defaultOrder = {
        Gtk4{},
        X11{},
    };

    if (auto xdgCurrentDesktop = std::getenv("XDG_CURRENT_DESKTOP")) {
        if (xdgCurrentDesktop == "GNOME"sv) {
            defaultOrder.insert(defaultOrder.begin(), Adwaita1{});
        }
    }

    return defaultOrder;
    /// [APlatformAbstractionOptions::defaultInitializationOrder]
}

APlatformAbstractionOptions& APlatformAbstractionOptions::inst() {
    static APlatformAbstractionOptions o;
    return o;
}


_unique<IPlatformAbstraction> APlatformAbstractionOptions::X11::operator()() {
    return std::make_unique<PlatformAbstractionX11>();
}

_unique<IPlatformAbstraction> APlatformAbstractionOptions::Gtk4::operator()() {
    return std::make_unique<PlatformAbstractionGtk>();
}

_unique<IPlatformAbstraction> APlatformAbstractionOptions::Adwaita1::operator()() {
    return std::make_unique<PlatformAbstractionAdw>();
}
