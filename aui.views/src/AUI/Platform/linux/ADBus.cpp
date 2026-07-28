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

//
// Created by alex2772 on 5/28/23.
//

#include "ADBus.h"
#include "AUI/Util/ARaiiHelper.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/unix/UnixIoThread.h"
#include "AUI/Platform/AWindow.h"

static constexpr auto LOG_TAG = "ADBus";

ADBus& ADBus::session() {
    static ADBus d;
    return d;
}

template <aui::invocable Callback>
void ADBus::throwExceptionOnError(Callback&& callback) {
    callback();
    if (dbus_error_is_set(mError.ptr())) {
        throw ADBus::Exception(mError->name);
    }
}
dbus_bool_t ADBus::addWatch(DBusWatch* watch, void* data) {
    auto bus = reinterpret_cast<ADBus*>(data);
    UnixIoThread::inst().registerCallback(
        dbus_watch_get_unix_fd(watch), UnixPollEvent::IN, [=](ABitField<UnixPollEvent> f) {
            unsigned int flags = 0;
            if (f.test(UnixPollEvent::IN)) {
                flags |= DBUS_WATCH_READABLE;
            }
            if (f.test(UnixPollEvent::OUT)) {
                flags |= DBUS_WATCH_WRITABLE;
            }
            dbus_watch_handle(watch, flags);
            bus->processMessages();
        });
    return true;
}
void removeWatch(DBusWatch* watch, void* data) {
    UnixIoThread::inst().unregisterCallback(dbus_watch_get_unix_fd(watch));
}
void watchToggled(DBusWatch* watch, void* data) {
    ALOG_DEBUG("ADBus") << "watchToggled";
}
void watchFree(void* data) {}

ADBus::ADBus() {
    throwExceptionOnError([&] { dbus_error_init(mError.ptr()); });
    throwExceptionOnError([&] { mConnection = dbus_bus_get(DBUS_BUS_SESSION, mError.ptr()); });
    ALogger::info(LOG_TAG) << "Connected to session bus";
    throwExceptionOnError([&] { dbus_bus_add_match(mConnection, "type='signal'", mError.ptr()); });
    if (!dbus_connection_set_watch_functions(mConnection, addWatch, removeWatch, watchToggled, this, watchFree)) {
        throw Exception("dbus_connection_set_watch_functions failed");
    }
}

ADBus::~ADBus() {
    throwExceptionOnError([&] { dbus_error_free(mError.ptr()); });
}

DBusHandlerResult ADBus::listener(DBusConnection* connection, DBusMessage* message, void* user_data) noexcept {
    auto listener = reinterpret_cast<MessageFilter*>(user_data);
    try {
        AUI_ASSERT(connection == listener->parent->mConnection);
        auto r = listener->function(message);
        return r;
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Exception occurred inside signal listener callback: " << e;
        return DBUS_HANDLER_RESULT_HANDLED;
    } catch (const std::bad_alloc& e) {
        ALogger::err(LOG_TAG) << "bad_alloc occurred inside signal listener callback: " << e;
        return DBUS_HANDLER_RESULT_NEED_MEMORY;
    } catch (const std::exception& e) {
        ALogger::err(LOG_TAG) << "Exception occurred inside signal listener callback: " << e;
        return DBUS_HANDLER_RESULT_HANDLED;
    }
}
void ADBus::deleter(void* userdata) noexcept {
    auto listener = reinterpret_cast<MessageFilter*>(userdata);
    auto listenerIt = std::find_if(
        listener->parent->mListeners.begin(), listener->parent->mListeners.end(),
        [&](const MessageFilter& lhs) { return &lhs == userdata; });

    if (listenerIt == listener->parent->mListeners.end()) {
        return;
    }
    listener->parent->mListeners.erase(listenerIt);
}

ADBus::MessageFilter::Ownership ADBus::addFilter(MessageFilter::Callback listener) {
    const auto it = mListeners.insert(mListeners.end(), { this, std::move(listener) });
    if (!dbus_connection_add_filter(mConnection, ADBus::listener, &(*it), deleter)) {
        throw Exception("dbus_connection_add_filter failed");
    }

    ADBus::MessageFilter::Ownership result(&(*it), [](MessageFilter* p) {
      dbus_connection_remove_filter(p->parent->mConnection, ADBus::listener, p);
    });

    return result;
}

void ADBus::processMessages() {
    dbus_connection_read_write(mConnection, 0);
    while (dbus_connection_get_dispatch_status(mConnection) == DBUS_DISPATCH_DATA_REMAINS)
        dbus_connection_dispatch(mConnection);
}

std::string_view ADBus::uniqueName() const noexcept { return dbus_bus_get_unique_name(mConnection); }

void aui::dbus::converter<aui::dbus::Variant>::iter_append(DBusMessageIter* iter, const Variant& t) {
    std::visit(
        [&]<typename T>(const T& containedValue) {
            DBusMessageIter sub;
            dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, converter<T>::signature.c_str(), &sub);
            AUI_DEFER { dbus_message_iter_close_container(iter, &sub); };
            aui::dbus::iter_append<T>(&sub, containedValue);
        },
        t);
}

aui::dbus::Variant aui::dbus::converter<aui::dbus::Variant>::iter_get(DBusMessageIter* iter) {
    if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_VARIANT) {
        throw AException("type error: expected variant, got '{:c}'"_format(got));
    }
    DBusMessageIter sub;
    dbus_message_iter_recurse(iter, &sub);
    switch (auto t = dbus_message_iter_get_arg_type(&sub)) {
        case DBUS_TYPE_INVALID:
            return aui::dbus::iter_get<std::nullopt_t>(&sub);
        case DBUS_TYPE_BYTE:
            return aui::dbus::iter_get<std::uint8_t>(&sub);
        case DBUS_TYPE_BOOLEAN:
            return aui::dbus::iter_get<bool>(&sub);
        case DBUS_TYPE_INT16:
            return aui::dbus::iter_get<std::int16_t>(&sub);
        case DBUS_TYPE_UINT16:
            return aui::dbus::iter_get<std::uint16_t>(&sub);
        case DBUS_TYPE_INT32:
            return aui::dbus::iter_get<std::int32_t>(&sub);
        case DBUS_TYPE_UINT32:
            return aui::dbus::iter_get<std::uint32_t>(&sub);
        case DBUS_TYPE_INT64:
            return aui::dbus::iter_get<std::int64_t>(&sub);
        case DBUS_TYPE_UINT64:
            return aui::dbus::iter_get<std::uint64_t>(&sub);
        case DBUS_TYPE_DOUBLE:
            return aui::dbus::iter_get<double>(&sub);
        case DBUS_TYPE_STRING:
            return aui::dbus::iter_get<std::string>(&sub);
        case DBUS_TYPE_OBJECT_PATH:
            return aui::dbus::iter_get<ObjectPath>(&sub);
        case DBUS_TYPE_ARRAY:
            return aui::dbus::iter_get<AVector<Unknown>>(&sub);
        default:
            throw AException("unable to process variant: {:c}"_format(t));
    }

    return std::nullopt;
}
aui::dbus::Unknown aui::dbus::converter<aui::dbus::Unknown>::iter_get(DBusMessageIter* iter) {
    return aui::dbus::Unknown(*iter);
}