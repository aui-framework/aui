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

#include <gtk/gtk.h>

#include "AUI/Common/AException.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "Gtk.h"

AMessageBox::ResultButton
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, Icon icon, Button b) {
    aui::detail::gtk::requires_gtk();

    AUI_DEFER { aui::detail::gtk::main_iterations(); };

    auto dialog = aui::ptr::make_unique_with_deleter(gtk_message_dialog_new(
        nullptr, static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL),
        [icon] {
            switch (icon) {
                case Icon::INFO:
                    return GTK_MESSAGE_INFO;
                case Icon::WARNING:
                    return GTK_MESSAGE_WARNING;
                case Icon::CRITICAL:
                    return GTK_MESSAGE_ERROR;
                default:
                case Icon::NONE:
                    return GTK_MESSAGE_OTHER;
            }
        }(),
        GTK_BUTTONS_NONE, "%s", message.toUtf8().data()), gtk_widget_destroy);

    if (b == Button::YES_NO_CANCEL || b == Button::YES_NO) {
        gtk_dialog_add_button(GTK_DIALOG(dialog.get()), "Yes"_i18n.toStdString().c_str(), GTK_RESPONSE_YES);
        gtk_dialog_add_button(GTK_DIALOG(dialog.get()), "No"_i18n.toStdString().c_str(), GTK_RESPONSE_NO);
    }

    if (b == Button::OK || b == Button::OK_CANCEL) {
        gtk_dialog_add_button(GTK_DIALOG(dialog.get()), "OK"_i18n.toStdString().c_str(), GTK_RESPONSE_OK);
    }

    if (b == Button::YES_NO_CANCEL || b == Button::OK_CANCEL) {
        gtk_dialog_add_button(GTK_DIALOG(dialog.get()), "Cancel"_i18n.toStdString().c_str(), GTK_RESPONSE_CANCEL);
    }

    gtk_window_set_title(GTK_WINDOW(dialog.get()), title.toStdString().c_str());

    auto response = gtk_dialog_run(GTK_DIALOG(dialog.get()));

    switch (response) {
        case GTK_RESPONSE_OK:
            return AMessageBox::ResultButton::OK;
        case GTK_RESPONSE_CANCEL:
            return AMessageBox::ResultButton::CANCEL;
        case GTK_RESPONSE_YES:
            return AMessageBox::ResultButton::YES;
        case GTK_RESPONSE_NO:
            return AMessageBox::ResultButton::NO;
        default:
            return AMessageBox::ResultButton::INVALID;
    }
}