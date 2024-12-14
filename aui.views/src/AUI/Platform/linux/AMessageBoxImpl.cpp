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

#include <libnotify/notify.h>

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

static void on_notification_closed(NotifyNotification *notification, gpointer user_data) {
    const auto *data = static_cast<AMessageBoxContext *>(user_data);
    g_main_loop_quit(data->loop);
}

static void on_action(NotifyNotification *notification, char *action, gpointer user_data) {
    auto *data = static_cast<AMessageBoxContext *>(user_data);

    if (strcmp(action, "yes") == 0) {
        data->result = AMessageBox::ResultButton::YES;
    } else if (strcmp(action, "no") == 0) {
        data->result = AMessageBox::ResultButton::NO;
    } else if (strcmp(action, "ok") == 0) {
        data->result = AMessageBox::ResultButton::OK;
    } else if (strcmp(action, "cancel") == 0) {
        data->result = AMessageBox::ResultButton::CANCEL;
    } else {
        data->result = AMessageBox::ResultButton::INVALID;
    }

    g_main_loop_quit(data->loop);
}

AMessageBox::ResultButton AMessageBox::show(
    AWindow *parent, const AString &title, const AString &message, AMessageBox::Icon icon, AMessageBox::Button b) {
    if (!notify_init("AUI_App")) {
        ALogger::err("AUI") << "Unable to initialize libnotify";
        return AMessageBox::ResultButton::INVALID;
    }

    NotifyNotification *notification =
        notify_notification_new(title.toStdString().c_str(), message.toStdString().c_str(), [icon] {
            switch (icon) {
                case Icon::INFO:
                    return "dialog-information";
                case Icon::WARNING:
                    return "dialog-warning";
                case Icon::CRITICAL:
                    return "dialog-error";
                default:
                    return "dialog-question";
            }
        }());

    AMessageBoxContext c {};
    c.result = ResultButton::INVALID;
    c.loop = g_main_loop_new(nullptr, FALSE);

    notify_notification_set_urgency(notification, NOTIFY_URGENCY_CRITICAL);

    if (b == Button::YES_NO_CANCEL || b == Button::YES_NO) {
        notify_notification_add_action(notification, "yes", "Yes"_i18n.toStdString().c_str(), on_action, &c, nullptr);

        notify_notification_add_action(notification, "no", "No"_i18n.toStdString().c_str(), on_action, &c, nullptr);

        if (b == Button::YES_NO_CANCEL) {
            notify_notification_add_action(
                notification, "cancel", "Cancel"_i18n.toStdString().c_str(), on_action, &c, nullptr);
        }
    } else if (b == Button::OK_CANCEL || b == Button::OK) {
        notify_notification_add_action(notification, "ok", "OK"_i18n.toStdString().c_str(), on_action, &c, nullptr);

        if (b == Button::OK_CANCEL) {
            notify_notification_add_action(
                notification, "cancel", "Cancel"_i18n.toStdString().c_str(), on_action, &c, nullptr);
        }
    }

    g_signal_connect(notification, "closed", G_CALLBACK(on_notification_closed), &c);

    if (!notify_notification_show(notification, nullptr)) {
        std::cerr << "Failed to show notification" << std::endl;
        g_object_unref(notification);
        notify_uninit();
        return ResultButton::INVALID;
    }

    g_main_loop_run(c.loop);

    g_object_unref(notification);
    g_main_loop_unref(c.loop);
    notify_uninit();

    return c.result;
}