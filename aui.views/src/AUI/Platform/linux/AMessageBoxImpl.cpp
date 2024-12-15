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
#include "AUI/View/AText.h"
#include "AUI/View/AButton.h"
#include "AUI/Thread/AEventLoop.h"
#include "AUI/Util/ARaiiHelper.h"

struct AMessageBoxContext {
    AMessageBox::ResultButton result;
    GMainLoop *loop;
};

static void on_response(GtkDialog *dialog, gint resp, gpointer user_data) {
    auto *ctx = static_cast<AMessageBoxContext *>(user_data);

    switch (resp) {
        case GTK_RESPONSE_OK:
            ctx->result = AMessageBox::ResultButton::OK;
            break;
        case GTK_RESPONSE_CANCEL:
            ctx->result = AMessageBox::ResultButton::CANCEL;
            break;
        case GTK_RESPONSE_YES:
            ctx->result = AMessageBox::ResultButton::YES;
            break;
        case GTK_RESPONSE_NO:
            ctx->result = AMessageBox::ResultButton::NO;
            break;
        default:
            ctx->result = AMessageBox::ResultButton::INVALID;
            break;
    }

    g_main_loop_quit(ctx->loop);
}

AMessageBox::ResultButton
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, Icon icon, Button b) {
    constexpr auto flags = static_cast<GtkDialogFlags>(GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);
    GtkWidget *dialog = gtk_message_dialog_new(
        nullptr, flags,
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
        GTK_BUTTONS_NONE, "%s", message.toUtf8().data());

    if (b == Button::YES_NO_CANCEL || b == Button::YES_NO) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes"_i18n.toStdString().c_str(), GTK_RESPONSE_YES);
        gtk_dialog_add_button(GTK_DIALOG(dialog), "No"_i18n.toStdString().c_str(), GTK_RESPONSE_NO);
    }

    if (b == Button::OK || b == Button::OK_CANCEL) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "OK"_i18n.toStdString().c_str(), GTK_RESPONSE_OK);
    }

    if (b == Button::YES_NO_CANCEL || b == Button::OK_CANCEL) {
        gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancel"_i18n.toStdString().c_str(), GTK_RESPONSE_CANCEL);
    }

    gtk_window_set_title(GTK_WINDOW(dialog), title.toStdString().c_str());

    AMessageBoxContext c {};
    c.result = ResultButton::INVALID;
    c.loop = g_main_loop_new(nullptr, FALSE);

    // Connect the response signal
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), &c);

    // Show the dialog
    gtk_widget_show(dialog);

    // Run the custom main loop
    g_main_loop_run(c.loop);

    // Clean up
    g_main_loop_unref(c.loop);
    gtk_widget_destroy(dialog);

    return c.result;
}