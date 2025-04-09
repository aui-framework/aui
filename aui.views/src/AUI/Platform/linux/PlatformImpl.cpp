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

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "ADBus.h"
#include "AUI/Logging/ALogger.h"
#include <AUI/Util/kAUI.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>


float APlatform::getDpiRatio()
{
    CommonRenderingContext::ensureXLibInitialized();
    if (CommonRenderingContext::ourDisplay == nullptr) return 1.f;

    static auto value = [] {
        char* resourceString = XResourceManagerString(CommonRenderingContext::ourDisplay);

        if (!resourceString) {
            return 1.f;
        }
        XrmInitialize();

        XrmValue value;
        char* type = nullptr;

        auto db = aui::ptr::make_unique_with_deleter(XrmGetStringDatabase(resourceString), XrmDestroyDatabase);

        if (XrmGetResource(db.get(), "Xft.dpi", "String", &type, &value)) {
            if (value.addr) {
                return float(atof(value.addr)) / 96.f;
            }
        }
        return 1.f;
    }();

    return value;
}

void APlatform::openUrl(const AUrl& url) {
    try {
        ADBus::inst().callBlocking("org.freedesktop.portal.Desktop",  // bus
                                   "/org/freedesktop/portal/desktop", // object
                                   "org.freedesktop.portal.OpenURI",  // interface
                                   "OpenURI",                         // method
                                   "", // parent
                                   url.full(),
                                   AMap<std::string, aui::dbus::Variant>()
        );
    } catch (const AException& e) {
        ALogger::err("APlatform") << "Failed to openUrl " << url.full() << ": " << e;
    }
}
