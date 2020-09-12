#if defined(_WIN32)
#include <Windows.h>
#include "AMessageBox.h"
#include "AWindow.h"
AMessageBox::Button AMessageBox::show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b)
{
    HWND window = parent ? parent->getNativeHandle() : nullptr;

    unsigned flags = 0;

    // Icons
    if (icon & I_INFO)
    {
        flags |= MB_ICONINFORMATION;
    }
    if (icon & I_WARNING)
    {
        flags |= MB_ICONWARNING;
    }
    if (icon & I_CRITICAL)
    {
        flags |= MB_ICONERROR;
    }

    // Flags
    if (b & B_OK)
    {
        flags |= MB_OK;
    }
    if (b & B_CANCEL)
    {
        flags |= MB_OKCANCEL;
    }

    switch (::MessageBox(window, message.c_str(), title.c_str(), flags)) {
    case IDOK:
        return B_OK;
    }
    return B_INVALID;
}
#elif defined(ANDROID)
#include "AMessageBox.h"
#include "AWindow.h"

AMessageBox::Button
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon,
                  AMessageBox::Button b) {

    return B_INVALID;
}
#else
#include <gtk/gtk.h>
#include "AMessageBox.h"
#include "AWindow.h"

AMessageBox::Button
AMessageBox::show(AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon,
                  AMessageBox::Button b) {
    unsigned iconFlags = 0;

    // Icons
    if (icon & I_INFO) {
        iconFlags |= GTK_MESSAGE_INFO;
    }
    if (icon & I_WARNING) {
        iconFlags |= GTK_MESSAGE_WARNING;
    }
    if (icon & I_CRITICAL) {
        iconFlags |= GTK_MESSAGE_ERROR;
    }

    unsigned buttonFlags = 0;

    // Flags
    if (b & B_OK) {
        buttonFlags |= GTK_BUTTONS_OK;
    }
    if (b & B_CANCEL) {
        buttonFlags |= GTK_BUTTONS_CANCEL;
    }

    auto dialog = gtk_message_dialog_new(nullptr,
                                         GTK_DIALOG_MODAL,
                                         (GtkMessageType)iconFlags,
                                         (GtkButtonsType)buttonFlags,
                                         "%ls", message.c_str());
    gtk_dialog_run(GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);

    return B_INVALID;
}

#endif