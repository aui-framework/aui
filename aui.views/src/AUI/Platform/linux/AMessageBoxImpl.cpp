// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AUI/Platform/AMessageBox.h"
#undef signals
#include <AUI/Util/kAUI.h>

AMessageBox::ResultButton
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon,
                  AMessageBox::Button b) {
    /*
    unsigned iconFlags = 0;

    aui_gtk_init();

    // Icons
    switch (icon) {
        case Icon::INFO:
            iconFlags = GTK_MESSAGE_INFO;
            break;
        case Icon::WARNING:
            iconFlags = GTK_MESSAGE_WARNING;
            break;
        case Icon::CRITICAL:
            iconFlags = GTK_MESSAGE_ERROR;
            break;
    }

    unsigned buttonFlags = 0;

    // Flags
    if (b == Button::OK) {
    }
    switch (b) {
        case Button::OK:
            buttonFlags = GTK_BUTTONS_OK;
            break;
        case Button::YES_NO:
            buttonFlags = GTK_BUTTONS_YES_NO;
            break;
        case Button::OK_CANCEL:
            buttonFlags = GTK_BUTTONS_CANCEL;
            break;
    }
    if (b == Button::YES_NO) {
        buttonFlags |= GTK_BUTTONS_YES_NO;
    } else if (b == Button::OK_CANCEL) {
        buttonFlags |= GTK_BUTTONS_CANCEL;
    }

    auto dialog = gtk_message_dialog_new(nullptr,
                                         GTK_DIALOG_MODAL,
                                         (GtkMessageType)iconFlags,
                                         (GtkButtonsType)buttonFlags,
                                         "%ls", title.c_str());
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%ls", message.c_str());
    gtk_window_set_keep_above(GTK_WINDOW(dialog), true);
    gtk_window_activate_focus(GTK_WINDOW(dialog));
    int resp = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));

    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
    switch (resp) {
        case GTK_RESPONSE_ACCEPT:
            return ResultButton::OK;
        case GTK_RESPONSE_YES:
            return ResultButton::YES;
    }
    return ResultButton::CANCEL;
     */
    return ResultButton::INVALID;
}