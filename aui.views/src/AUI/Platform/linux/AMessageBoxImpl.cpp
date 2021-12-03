/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AUI/Platform/AMessageBox.h"
#include <gtk/gtk.h>
#include <AUI/Util/kAUI.h>
#include "AMessageBox.h"
#include "AWindow.h"
#include <AUI/Util/kAUI.h>

extern void aui_gtk_init();
AMessageBox::ResultButton
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon,
                  AMessageBox::Button b) {
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
                                         "%ls", message.c_str());
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
}