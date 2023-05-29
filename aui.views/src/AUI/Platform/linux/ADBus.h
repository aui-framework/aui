//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Util/APimpl.h>
#include <AUI/Util/ARaiiHelper.h>
#include "AUI/Common/AException.h"
#include <dbus/dbus.h>

struct DBusError;
struct DBusConnection;

namespace aui::dbus {
    using Variant = std::variant<
            std::nullopt_t,      // DBUS_TYPE_INVALID
            std::uint8_t,        // DBUS_TYPE_BYTE
            bool,                // DBUS_TYPE_BOOLEAN
            std::int16_t,        // DBUS_TYPE_INT16
            std::uint16_t,       // DBUS_TYPE_UINT16
            std::int32_t,        // DBUS_TYPE_INT32
            std::uint32_t,       // DBUS_TYPE_UINT32
            std::int64_t,        // DBUS_TYPE_INT64
            std::uint64_t,       // DBUS_TYPE_UINT64
            double,              // DBUS_TYPE_DOUBLE
            std::string          // DBUS_TYPE_STRING
    >;

    template<typename T>
    struct converter;

    template<typename T>
    concept convertible = requires(T t) {
        { converter<T>::iter_append(static_cast<DBusMessageIter*>(nullptr), t) };
        { converter<T>::signature } -> aui::convertible_to<std::string>;
    };

    template<typename T>
    concept convertible_or_void = convertible<T> || std::is_void_v<T>;


    template<convertible T>
    void iter_append(DBusMessageIter* iter, const T& value) {
        converter<T>::iter_append(iter, value);
    }

    namespace impl {
        template<typename T, char dbusType = DBUS_TYPE_INVALID>
        struct basic_converter {
            static inline std::string signature = fmt::format("{}", dbusType);

            static void iter_append(DBusMessageIter* iter, const T& t) {
                dbus_message_iter_append_basic(iter, dbusType, &t);
            }
        };
    }

    template<> struct converter<std::nullopt_t> { // -> DBUS_TYPE_INVALID
        static inline std::string signature = DBUS_TYPE_INVALID_AS_STRING;

        static void iter_append(DBusMessageIter* iter, std::nullopt_t) {
            int v = 0;
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INVALID, &v);
        }
    };

    template<> struct converter<std::uint8_t  >: impl::basic_converter<std::uint8_t  , DBUS_TYPE_BYTE> {};
    template<> struct converter<bool          >: impl::basic_converter<bool          , DBUS_TYPE_BOOLEAN> {};
    template<> struct converter<std::int16_t  >: impl::basic_converter<std::int16_t  , DBUS_TYPE_INT16> {};
    template<> struct converter<std::uint16_t >: impl::basic_converter<std::uint16_t , DBUS_TYPE_UINT16> {};
    template<> struct converter<std::int32_t  >: impl::basic_converter<std::int32_t  , DBUS_TYPE_INT32> {};
    template<> struct converter<std::uint32_t >: impl::basic_converter<std::uint32_t , DBUS_TYPE_UINT32> {};
    template<> struct converter<std::int64_t  >: impl::basic_converter<std::int64_t  , DBUS_TYPE_INT64> {};
    template<> struct converter<std::uint64_t >: impl::basic_converter<std::uint64_t , DBUS_TYPE_UINT64> {};
    template<> struct converter<double        >: impl::basic_converter<double        , DBUS_TYPE_DOUBLE> {};
    template<> struct converter<const char*   >: impl::basic_converter<const char*   , DBUS_TYPE_STRING> {};
    template<> struct converter<std::string>: converter<const char*> {
        static void iter_append(DBusMessageIter* iter, const std::string& t) {
            converter<const char*>::iter_append(iter, t.c_str());
        }
    };

    template<> struct converter<AString>: converter<std::string> {
        static void iter_append(DBusMessageIter* iter, const AString& t) {
            converter<std::string>::iter_append(iter, t.toStdString());
        }
    };
    template<std::size_t N> struct converter<char[N]>: converter<const char*> {};


    template<convertible T>
    struct converter<AVector<T>> {
        static inline std::string signature = fmt::format("a{}", converter<T>::signature);

        static void iter_append(DBusMessageIter* iter, const AVector<T>& t) {
            DBusMessageIter sub;
            const auto s = converter<T>::signature.c_str();
            if (!dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, s, &sub)) {
                throw AException("dbus_message_iter_open_container failed");
            }
            ARaiiHelper h = [&] {
                dbus_message_iter_close_container(iter, &sub);
            };

            for (const auto& v : t) {
                aui::dbus::iter_append(&sub, v);
            }
        }
    };

    template<convertible... Types>
    struct converter<std::tuple<Types...>> {
        static inline std::string signature = "{" + (... + converter<Types>::signature) + "}";

        static void iter_append(DBusMessageIter* iter, const std::tuple<Types...>& t) {
            DBusMessageIter sub;
            if (!dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, nullptr, &sub)) {
                throw AException("dbus_message_iter_open_container failed");
            }
            ARaiiHelper h = [&] {
                dbus_message_iter_close_container(iter, &sub);
            };

            std::apply([&](const auto&... args){
                (..., aui::dbus::iter_append(&sub, args));
            }, t);
        }
    };
    template<>
    struct converter<Variant> {
        static inline std::string signature = "v";

        static void iter_append(DBusMessageIter* iter, const Variant & t) {
        }
    };
}

/**
 * @brief IPC on freedesktop linux
 * @ingroup views
 */
class API_AUI_VIEWS ADBus: public aui::noncopyable {
public:

    /**
     * @brief Exception thrown on dbus errors.
     */
    class Exception: public AException {
        using AException::AException;
    };

    static ADBus& inst();

    template<aui::dbus::convertible_or_void Return = void, aui::dbus::convertible... Args>
    Return callBlocking(const AString& bus,
                        const AString& path,
                        const AString& interface,
                        const AString& method,
                        const Args&... args) {
        auto msg = aui::ptr::make_unique_with_deleter(dbus_message_new_method_call(bus.toStdString().c_str(),
                                                                                   path.toStdString().c_str(),
                                                                                   interface.toStdString().c_str(),
                                                                                   method.toStdString().c_str()),
                                                      dbus_message_unref);

        auto formatError = [&](const AString& info) {
            return "unable to invoke {};{};{};{}: {}"_format(bus, path, interface, method, info);
        };

        if (!msg) {
            throw ADBus::Exception(formatError("message is null"));
        }

        DBusMessageIter dbusArgs;
        dbus_message_iter_init_append(msg.get(), &dbusArgs);

        aui::parameter_pack::for_each([&](const auto& v) {
            aui::dbus::iter_append(&dbusArgs, v);
        }, args...);

        auto pending = aui::ptr::make_unique_with_deleter([&] {
            DBusPendingCall* pending;
            if (!dbus_connection_send_with_reply(mConnection, msg.get(), &pending, -1)) { // -1 is default timeout
                throw ADBus::Exception(formatError("dbus_connection_send_with_reply failed"));
            }
            return pending;
        }(), dbus_pending_call_unref);

        // block until we receive a reply
        dbus_pending_call_block(pending.get());

        // get the reply message
        msg.reset(dbus_pending_call_steal_reply(pending.get()));

        if (dbus_message_get_type(msg.get()) == DBUS_MESSAGE_TYPE_ERROR) {
            if (auto e = dbus_message_get_error_name(msg.get())) {
                throw ADBus::Exception(formatError(e));
            }
            throw ADBus::Exception(formatError("dbus replied unknown error"));
        }

        if constexpr (!std::is_void_v<Return>) {

        }
    }


private:

    template<aui::invocable Callback>
    void throwExceptionOnError(Callback&& callback);

    ADBus();
    ~ADBus();

    aui::fast_pimpl<DBusError, sizeof(void*) * 3 + 20, alignof(void*)> mError;
    DBusConnection* mConnection = nullptr;
};
