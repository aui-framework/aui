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

namespace {
/// `current_folder` of the portal: a NUL-terminated byte array.
AVector<std::uint8_t> portalPath(const APath& path) {
    AVector<std::uint8_t> bytes;
    for (const AByteBufferView buffer(path.data(), path.size()); const auto& byte : buffer) {
        bytes.push_back(byte);
    }
    bytes.push_back(0);
    return bytes;
}

/// A method of org.freedesktop.portal.FileChooser (`OpenFile`, `SaveFile`): the first uri the user chose, or an empty
/// path if they cancelled.
AFuture<APath> fileChooser(
    ASurface* parent, const std::string& method, const AString& title, const APath& startingLocation,
    AMap<std::string, aui::dbus::Variant> options) {
    AUI_NULLSAFE(parent)->blockUserInput();
    if (!startingLocation.empty()) {
        options.insert_or_assign("current_folder", portalPath(startingLocation));
    }

    auto p = *ADBus::session().callWithResult<aui::dbus::ObjectPath>(
        "org.freedesktop.portal.Desktop",       // bus
        "/org/freedesktop/portal/desktop",      // object
        "org.freedesktop.portal.FileChooser",   // interface
        method,                                 // method
        "",                                     // parent_window
        title,                                  // title,
        options                                 // options
    );

    std::shared_ptr<ADBus::MessageFilter> unsubscribe;
    auto f = AFuture<APath>::create([&](AFuture<APath>::Resolver resolver) {
      unsubscribe = ADBus::session().addSignalListener(
        std::move(p), "org.freedesktop.portal.Request", "Response",
        [resolver](std::uint32_t response, AMap<std::string, aui::dbus::Variant> results) {
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
                            resolver.supplyValue(result.path());
                            return;
                        }
                    }
                }
                resolver.supplyValue({});
            } catch (...) {
                resolver.supplyException();
                throw;
            }
        });
      resolver.onCancel([parent, unsubscribe] {
          AThread::main()->enqueue([parent] { AUI_NULLSAFE(parent)->blockUserInput(false); });
      });
    });

    f.onFinally([parent, unsubscribe = std::move(unsubscribe)] {
        AUI_NULLSAFE(parent)->blockUserInput(false);
    });

    return f;
}
}   // namespace

AFuture<APath>
ADesktop::browseForFile(ASurface *parent, const APath &startingLocation, const AVector<FileExtension>& extensions) {
    return fileChooser(parent, "OpenFile", "Open File"_i18n, startingLocation, {});
}

AFuture<APath> ADesktop::browseForDir(ASurface *parent, const APath &startingLocation) {
    AMap<std::string, aui::dbus::Variant> options;
    options.insert_or_assign("directory", true);
    return fileChooser(parent, "OpenFile", "Open Folder"_i18n, startingLocation, std::move(options));
}

AFuture<APath> ADesktop::browseForSaveFile(
    ASurface* parent, const APath& startingLocation, const AString& suggestedName,
    const AVector<FileExtension>& extensions) {
    AMap<std::string, aui::dbus::Variant> options;
    if (!suggestedName.empty()) {
        options.insert_or_assign("current_name", suggestedName.toStdString());
    }
    return fileChooser(parent, "SaveFile", "Save File"_i18n, startingLocation, std::move(options));
}

_<IDrawable> ADesktop::iconOfFile(const APath &file) { return nullptr; }

void ADesktop::playSystemSound(ADesktop::SystemSound s) {}
