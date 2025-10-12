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

#include <AUI/Platform/APlatform.h>
#include <AUI/Common/AString.h>
#include <AUI/IO/APath.h>
#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Platform/linux/dbus-cxx/dbus-cxx.h>
#include <AUI/Logging/ALogger.h>
#include "IPlatformAbstraction.h"
#include <AUI/Util/kAUI.h>


float APlatform::getDpiRatio() {
    return IPlatformAbstraction::current().platformGetDpiRatio();
}

void APlatform::openUrl(const AUrl& url) {
    try {
        ADBus::session().call(
            "org.freedesktop.portal.Desktop",    // bus
            "/org/freedesktop/portal/desktop",   // object
            "org.freedesktop.portal.OpenURI",    // interface
            "OpenURI",                           // method
            "",                                  // parent
            url.full(), AMap<std::string, aui::dbus::Variant>());
    } catch (const AException& e) {
        ALogger::err("APlatform") << "Failed to openUrl " << url.full() << ": " << e;
    }
}
