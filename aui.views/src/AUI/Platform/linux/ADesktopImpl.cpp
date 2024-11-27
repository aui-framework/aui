/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/ADesktop.h>
#include <AUI/Util/ARaiiHelper.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <AUI/Util/kAUI.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/Platform/ADesktop.h>
#include <AUI/Platform/ACursor.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/CommonRenderingContext.h>
#include "ADBus.h"

#undef signals

glm::ivec2 ADesktop::getMousePosition()
{
    glm::ivec2 p;
    Window w;
    int unused1;
    unsigned unused2;
    XQueryPointer(CommonRenderingContext::ourDisplay, XRootWindow(CommonRenderingContext::ourDisplay, 0), &w, &w, &p.x, &p.y, &unused1, &unused1, &unused2);
    return p;
}

void ADesktop::setMousePos(const glm::ivec2& pos)
{
    auto rootWindow = XRootWindow(CommonRenderingContext::ourDisplay, 0);
    XSelectInput(CommonRenderingContext::ourDisplay, rootWindow, KeyReleaseMask);
    XWarpPointer(CommonRenderingContext::ourDisplay, None, rootWindow, 0, 0, 0, 0, pos.x, pos.y);
    XFlush(CommonRenderingContext::ourDisplay);
}


AFuture<APath> ADesktop::browseForFile(AWindowBase* parent, const APath& startingLocation, const AVector<FileExtension>& extensions) {
    parent->blockUserInput();
    auto p = ADBus::inst().callBlocking<aui::dbus::ObjectPath>("org.freedesktop.portal.Desktop",     // bus
                                                               "/org/freedesktop/portal/desktop",    // object
                                                               "org.freedesktop.portal.FileChooser", // interface
                                                               "OpenFile",                           // method
                                                               "", // parent
                                                               "Open File"_i18n,
                                                               AMap<std::string, aui::dbus::Variant>()
    );
    AFuture<APath> f;
    ADBus::inst().addSignalListener(std::move(p),
                                    "org.freedesktop.portal.Request",
                                    "Response",
                                    [f](std::uint32_t response, AMap<std::string, aui::dbus::Variant> results) {
        try {
            if (auto c = results.contains("uris")) {
                if (auto p = std::get_if<AVector<aui::dbus::Unknown>>(&c->second)) {
                    if (!p->empty()) {
                        AUrl result = p->first().as<AString>();
                        f.supplyValue(result.path());
                        return;
                    }
                }
            }
            throw AException("path not specified");
        } catch (...) {
            f.supplyException();
            throw;
        }
    });
    return f;
}

AFuture<APath> ADesktop::browseForDir(AWindowBase* parent, const APath& startingLocation) {
    parent->blockUserInput();
    return async {
        /*
        ARaiiHelper windowUnblocker = [&] {
            parent->getThread()->enqueue([parent] {
                parent->blockUserInput(false);
            });
        };
        aui_gtk_init();
        GtkWidget *dialog;
        GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
        gint res;

        dialog = gtk_file_chooser_dialog_new ("Open folder"_i18n.toStdString().c_str(),
                                              nullptr,
                                              action,
                                              "Cancel"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_CANCEL,
                                              "Open"_i18n.toStdString().c_str(),
                                              GTK_RESPONSE_ACCEPT,
                                              nullptr);


        gtk_file_chooser_set_current_folder(reinterpret_cast<GtkFileChooser*>(dialog), startingLocation.toStdString().c_str());

        gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
        gtk_window_activate_focus(GTK_WINDOW(dialog));

        res = gtk_dialog_run (GTK_DIALOG (dialog));
        if (res == GTK_RESPONSE_ACCEPT)
        {
            char *filename;
            GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
            filename = gtk_file_chooser_get_filename (chooser);
            APath f = filename;
            g_free(filename);
            gtk_widget_destroy (dialog);

            while (gtk_events_pending()) {
                gtk_main_iteration();
            }
            return f;
        }

        gtk_widget_destroy (dialog);

        while (gtk_events_pending()) {
            gtk_main_iteration();
        }*/
        return APath{};
    };
}

_<IDrawable> ADesktop::iconOfFile(const APath& file) {
    return nullptr;
}

void ADesktop::playSystemSound(ADesktop::SystemSound s) {

}
