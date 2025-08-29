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

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ACursor.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/CommonRenderingContext.h>
#include "ADBus.h"
#include "IPlatformAbstraction.h"

#undef signals

glm::ivec2 ADesktop::getMousePosition() {
    return IPlatformAbstraction::current().desktopGetMousePosition();
}

void ADesktop::setMousePos(const glm::ivec2 &pos) {
    IPlatformAbstraction::current().desktopSetMousePosition(pos);
}

AFuture<APath>
ADesktop::browseForFile(AWindowBase *parent, const APath &startingLocation, const AVector<FileExtension>& extensions) {
    AUI_NULLSAFE(parent)->blockUserInput();
    AUI_DEFER { AUI_NULLSAFE(parent)->blockUserInput(false); };

    AMap<std::string, aui::dbus::Variant> options;

    if (!startingLocation.empty()) {
        AVector<std::uint8_t> locationBytes;
        for (const AByteBufferView buffer(startingLocation.data(), startingLocation.sizeBytes()); const auto& byte : buffer) {
            locationBytes.push_back(byte);
        }

        options.insert_or_assign("current_folder", locationBytes);
    }

    auto p = ADBus::inst().callBlocking<aui::dbus::ObjectPath>(
        "org.freedesktop.portal.Desktop",       // bus
        "/org/freedesktop/portal/desktop",      // object
        "org.freedesktop.portal.FileChooser",   // interface
        "OpenFile",                             // method
        "",                                     // parent_window
        "Open File"_i18n,                       // title,
        options                                 // options
    );

    AFuture<APath> f;

    const auto unsubscribe = ADBus::inst().addSignalListener(
        std::move(p), "org.freedesktop.portal.Request", "Response",
        [f](std::uint32_t response, AMap<std::string, aui::dbus::Variant> results) {
            // The following results get returned via the :ref:`org.freedesktop.portal.Request::Response` signal:
            //
            //  * ``uris`` (``as``)
            //
            //    An array of strings containing the uris of the selected files. All URIs
            //    have the ``file://`` scheme.
            //
            //  * ``choices`` (``a(ss)``)
            //
            //    An array of pairs of strings, the first string being the ID of a
            //    combobox that was passed into this call, the second string being
            //    the selected option.
            //
            //    Example: ``[('encoding', 'utf8'), ('reencode', 'true')]``
            //
            //  * ``current_filter`` (``(sa(us))``)
            //
            //    The filter that was selected. This may match a filter in the
            //    filter list or another filter that was applied unconditionally.

            try {
                if (const auto c = results.contains("uris")) {
                    if (const auto t = std::get_if<AVector<aui::dbus::Unknown> >(&c->second)) {
                        if (AString path; !t->empty() && t->first().as(path)) {
                            const AUrl result = path;
                            f.supplyValue(result.path());
                            return;
                        }
                    }
                }
                f.supplyValue({});
            } catch (...) {
                f.supplyException();
                throw;
            }
        });

    f.onFinally([parent, unsubscribe]() {
        AThread::current()->enqueue([=] {
          unsubscribe();
          AUI_NULLSAFE(parent)->blockUserInput(false);
        });
    });

    return f;
}

AFuture<APath> ADesktop::browseForDir(AWindowBase *parent, const APath &startingLocation) {
    AUI_NULLSAFE(parent)->blockUserInput();

    AMap<std::string, aui::dbus::Variant> options;
    options.insert_or_assign("directory", true);

    if (!startingLocation.empty()) {
        AVector<std::uint8_t> locationBytes;
        for (const AByteBufferView buffer(startingLocation.data(), startingLocation.sizeBytes()); const auto &byte : buffer) {
            locationBytes.push_back(byte);
        }

        options.insert_or_assign("current_folder", locationBytes);
    }

    auto p = ADBus::inst().callBlocking<aui::dbus::ObjectPath>(
        "org.freedesktop.portal.Desktop",       // bus
        "/org/freedesktop/portal/desktop",      // object
        "org.freedesktop.portal.FileChooser",   // interface
        "OpenFile",                             // method
        "",                                     // parent_window
        "Open Folder"_i18n,                     // title,
        options                                 // options
    );

    AFuture<APath> f;

    const auto unsubscribe = ADBus::inst().addSignalListener(
        std::move(p), "org.freedesktop.portal.Request", "Response",
        [f](std::uint32_t response, AMap<std::string, aui::dbus::Variant> results) {
            try {
                if (const auto c = results.contains("uris")) {
                    if (const auto t = std::get_if<AVector<aui::dbus::Unknown> >(&c->second)) {
                        if (AString path; !t->empty() && t->first().as(path)) {
                            const AUrl result = path;
                            f.supplyValue(result.path());
                            return;
                        }
                    }
                }
                f.supplyValue({});
            } catch (...) {
                f.supplyException();
                throw;
            }
        });

    f.onFinally([parent, unsubscribe]() {
        unsubscribe();
        AUI_NULLSAFE(parent)->blockUserInput(false);
    });

    return f;
}

_<IDrawable> ADesktop::iconOfFile(const APath &file) { return nullptr; }

void ADesktop::playSystemSound(ADesktop::SystemSound s) {}
