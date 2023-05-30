﻿//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    char *resourceString = XResourceManagerString(CommonRenderingContext::ourDisplay);
    XrmDatabase db;
    XrmValue value;
    char *type = NULL;
    float dpi = 1.f;

    do_once {
        XrmInitialize();
    };

    db = XrmGetStringDatabase(resourceString);

    if (resourceString) {
        if (XrmGetResource(db, "Xft.dpi", "String", &type, &value)) {
            if (value.addr) {
                dpi = atof(value.addr) / 96.f;
            }
        }
    }

    return dpi;
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