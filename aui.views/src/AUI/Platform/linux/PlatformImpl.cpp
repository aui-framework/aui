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
#include <AUI/Platform/linux/ADBus.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Platform/linux/IPlatformAbstraction.h>
#include <AUI/Util/kAUI.h>

static void openUrlViaPortal(const AUrl& url) {
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

void APlatform::openUrl(const AUrl& url) {
    // xdg-desktop-portal's OpenURI does not handle directories (replies with an error which is never observed), so
    // ask the file manager directly via org.freedesktop.FileManager1 (Nautilus, Dolphin, Thunar, Nemo, ...).
    if (url.schema() == "file" && APath(url.path()).isDirectoryExists()) {
        try {
            *ADBus::session()
                .callWithResult<void>(
                    "org.freedesktop.FileManager1",     // bus
                    "/org/freedesktop/FileManager1",    // object
                    "org.freedesktop.FileManager1",     // interface
                    "ShowFolders",                      // method
                    AVector<std::string>{ url.full().toStdString() },
                    std::string{})                      // startup id
                ;
            return;
        } catch (const AException& e) {
            ALogger::warn("APlatform") << "FileManager1 is not available, falling back to portal: " << e;
        }
    }
    openUrlViaPortal(url);
}
