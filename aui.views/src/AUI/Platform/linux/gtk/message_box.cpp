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

#include "PlatformAbstractionGtk.h"
#include <AUI/i18n/AI18n.h>

template<std::size_t N>
static auto toC(const std::string(&rawArray)[N]) {
    std::array<const char*, N + 1> result{};
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = rawArray[i].c_str();
    }
    result[N] = nullptr;
    return result;
}

AMessageBox::ResultButton PlatformAbstractionGtk::messageBoxShow(
    AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b) {

    auto dialog = AGlibPtr(gtk_alert_dialog_new("%s", message.toStdString().c_str()));
    gtk_alert_dialog_set_modal(dialog, true);
    gtk_alert_dialog_set_detail(dialog, title.toStdString().c_str());

    std::array<AMessageBox::ResultButton, 4> responses{};

    switch (b) {
        case AMessageBox::Button::OK:
            responses[0] = AMessageBox::ResultButton::OK;
            break;

        case AMessageBox::Button::OK_CANCEL: {
            responses[0] = AMessageBox::ResultButton::OK;
            responses[1] = AMessageBox::ResultButton::CANCEL;

            std::string buttons[] = {
                "OK"_i18n.toStdString(),
                "Cancel"_i18n.toStdString(),
            };

            gtk_alert_dialog_set_buttons(
                dialog, toC(buttons).data());
            break;
        }

        case AMessageBox::Button::YES_NO: {
            responses[0] = AMessageBox::ResultButton::YES;
            responses[1] = AMessageBox::ResultButton::NO;

            std::string buttons[] = {
                "Yes"_i18n.toStdString(),
                "No"_i18n.toStdString(),
            };

            gtk_alert_dialog_set_buttons(
                dialog, toC(buttons).data());
            break;
        }

        case AMessageBox::Button::YES_NO_CANCEL: {
            responses[0] = AMessageBox::ResultButton::YES;
            responses[1] = AMessageBox::ResultButton::NO;
            responses[2] = AMessageBox::ResultButton::CANCEL;

            std::string buttons[] = {
                "Yes"_i18n.toStdString(),
                "No"_i18n.toStdString(),
                "Cancel"_i18n.toStdString(),
            };

            gtk_alert_dialog_set_buttons(
                dialog, toC(buttons).data());
            break;
        }
    }

    struct State {
        GtkAlertDialog* dialog;
        int button = 0;
        bool isComplete = false;
    } state = {
        .dialog = dialog
    };

    gtk_alert_dialog_choose(
        dialog, parent ? nativeHandle(*parent) : nullptr, nullptr,
        (GAsyncReadyCallback) +
            [](GtkAlertDialog* source_object, GAsyncResult* res, State* data) {
                data->button = gtk_alert_dialog_choose_finish (GTK_ALERT_DIALOG(data->dialog), res, nullptr);
                data->isComplete = true;
            },
        &state);

    // this is sooo bad
    while (!state.isComplete) {
        dynamic_cast<PlatformAbstractionGtk&>(IPlatformAbstraction::current()).windowManagerIteration();
    }

    return responses[state.button];
}
