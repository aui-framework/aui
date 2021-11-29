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

#include "AMessageBox.h"

#if AUI_PLATFORM_WIN
#include <windows.h>
#include "AMessageBox.h"
#include "AWindow.h"
AMessageBox::ResultButton AMessageBox::show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b)
{
    HWND window = parent ? parent->getNativeHandle() : nullptr;

    long flags = 0;

    // Icons
    switch (icon)
    {
        case Icon::INFO:
            flags |= MB_ICONINFORMATION;
            break;
        case Icon::WARNING:
            flags |= MB_ICONWARNING;
            break;
        case Icon::CRITICAL:
            flags |= MB_ICONSTOP;
            break;
    }


    // Buttons
    switch (b) {
        case Button::OK:
            flags |= MB_OK;
            break;

        case Button::OK_CANCEL:
            flags |= MB_OKCANCEL;
            break;

        case Button::YES_NO:
            flags |= MB_YESNO;
            break;
    }

    switch (::MessageBox(window, message.c_str(), title.c_str(), flags)) {
        case IDOK:
            return ResultButton::OK;
        case IDCANCEL:
            return ResultButton::CANCEL;
        case IDYES:
            return ResultButton::YES;
        case IDNO:
            return ResultButton::NO;
    }
    return ResultButton::INVALID;
}
#elif AUI_PLATFORM_ANDROID

#include <AUI/Platform/OSAndroid.h>
#include "AMessageBox.h"
#include "AWindow.h"

AMessageBox::ResultButton show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b) {

    auto j = AAndroid::getJNI();
    auto klazzAUI = j->FindClass("ru/alex2772/aui/AUI");
    auto methodShowMessageBox = j->GetStaticMethodID(klazzAUI, "showMessageBox", "(Ljava/lang/String;Ljava/lang/String;)V");
    auto strTitle = j->NewStringUTF(title.toStdString().c_str());
    auto strMessage = j->NewStringUTF(message.toStdString().c_str());

    j->CallStaticVoidMethod(klazzAUI, methodShowMessageBox, strTitle, strMessage);

    j->DeleteLocalRef(strTitle);
    j->DeleteLocalRef(strMessage);

    return AMessageBox::ResultButton::INVALID;
}

#elif AUI_PLATFORM_APPLE

AMessageBox::ResultButton AMessageBox::show(AWindow *parent, const AString &title, const AString &message, Icon icon, Button b) {
    // TODO apple
    return AMessageBox::ResultButton::INVALID;
}

#else
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

#endif