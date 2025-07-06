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

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Traits/concepts.h>
#include <AUI/Util/APimpl.h>
#include <AUI/Util/ARaiiHelper.h>
#include "AUI/Common/AException.h"
#include <dbus/dbus.h>
#include <list>

struct DBusError;
struct DBusConnection;

namespace aui::dbus {
    class ObjectPath: public std::string {
        using std::string::string;
    };

    template<typename T>
    struct converter;

    template<typename T>
    concept convertible = requires(T t) {
        { converter<T>::iter_append(static_cast<DBusMessageIter*>(nullptr), t) };
        { converter<T>::signature } -> aui::convertible_to<std::string>;
    };

    struct Unknown {
    public:
        explicit Unknown(const DBusMessageIter& mIter) : mIter(mIter) {}

        template<convertible T>
        T as();

        template<convertible T>
        bool as(T& v);

    private:
        DBusMessageIter mIter;
    };

    using VariantImpl = std::variant<
            std::nullopt_t,         // DBUS_TYPE_INVALID
            std::uint8_t,           // DBUS_TYPE_BYTE
            bool,                   // DBUS_TYPE_BOOLEAN
            std::int16_t,           // DBUS_TYPE_INT16
            std::uint16_t,          // DBUS_TYPE_UINT16
            std::int32_t,           // DBUS_TYPE_INT32
            std::uint32_t,          // DBUS_TYPE_UINT32
            std::int64_t,           // DBUS_TYPE_INT64
            std::uint64_t,          // DBUS_TYPE_UINT64
            double,                 // DBUS_TYPE_DOUBLE
            std::string,            // DBUS_TYPE_STRING
            ObjectPath,             // DBUS_TYPE_OBJECT_PATH
            AVector<Unknown>,       // DBUS_TYPE_ARRAY
            AVector<std::uint8_t>   // DBUS_TYPE_ARRAY
    >;
    struct Variant: VariantImpl {
        using VariantImpl::variant;
        Variant(): VariantImpl(std::nullopt) {}
    };

    template<typename T>
    concept convertible_or_void = convertible<T> || std::is_void_v<T>;


    template<convertible T>
    void iter_append(DBusMessageIter* iter, const T& value) {
        converter<T>::iter_append(iter, value);
    }
    template<convertible T>
    T iter_get(DBusMessageIter* iter) {
        return converter<T>::iter_get(iter);
    }

    template<convertible T>
    T Unknown::as() {
        return aui::dbus::iter_get<T>(&mIter);
    }

    template<convertible T>
    bool Unknown::as(T& v) {
        if (auto got = dbus_message_iter_get_arg_type(&mIter); got != converter<T>::signature[0]) {
            return false;
        }

        v = aui::dbus::iter_get<T>(&mIter);
        return true;
    }


    namespace impl {
        template<typename T, char dbusType = DBUS_TYPE_INVALID>
        struct basic_converter {
            static inline std::string signature = fmt::format("{}", dbusType);

            static void iter_append(DBusMessageIter* iter, const T& t) {
                dbus_message_iter_append_basic(iter, dbusType, &t);
            }
            static T iter_get(DBusMessageIter* iter) {
                if (auto got = dbus_message_iter_get_arg_type(iter); got != dbusType) {
                    throw AException("type error: expected '{:c}', got '{:c}'"_format(dbusType, got));
                }
                T t;
                dbus_message_iter_get_basic(iter, &t);
                return t;
            }
        };
    }

    template<> struct converter<std::nullopt_t> { // -> DBUS_TYPE_INVALID
        static inline std::string signature = DBUS_TYPE_INVALID_AS_STRING;

        static void iter_append(DBusMessageIter* iter, std::nullopt_t) {
            int v = 0;
            dbus_message_iter_append_basic(iter, DBUS_TYPE_INVALID, &v);
        }

        static auto iter_get(DBusMessageIter* iter) {
            if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_INVALID) {
                throw AException("type error: expected '{:c}', got '{:c}'"_format(DBUS_TYPE_INVALID, got));
            }
            return std::nullopt;
        }
    };

    template<> struct converter<std::uint8_t  >: impl::basic_converter<std::uint8_t  , DBUS_TYPE_BYTE> {};
    template<> struct converter<std::int16_t  >: impl::basic_converter<std::int16_t  , DBUS_TYPE_INT16> {};
    template<> struct converter<std::uint16_t >: impl::basic_converter<std::uint16_t , DBUS_TYPE_UINT16> {};
    template<> struct converter<std::int32_t  >: impl::basic_converter<std::int32_t  , DBUS_TYPE_INT32> {};
    template<> struct converter<std::uint32_t >: impl::basic_converter<std::uint32_t , DBUS_TYPE_UINT32> {};
    template<> struct converter<std::int64_t  >: impl::basic_converter<std::int64_t  , DBUS_TYPE_INT64> {};
    template<> struct converter<std::uint64_t >: impl::basic_converter<std::uint64_t , DBUS_TYPE_UINT64> {};
    template<> struct converter<double        >: impl::basic_converter<double        , DBUS_TYPE_DOUBLE> {};
    template<> struct converter<const char*   >: impl::basic_converter<const char*   , DBUS_TYPE_STRING> {};
    template<> struct converter<bool> {
        static inline std::string signature = DBUS_TYPE_BOOLEAN_AS_STRING;

        static void iter_append(DBusMessageIter* iter, const bool& t) {
            impl::basic_converter<std::int32_t, DBUS_TYPE_BOOLEAN>::iter_append(iter, t ? 1 : 0);
        }
        static bool iter_get(DBusMessageIter* iter) {
            return impl::basic_converter<std::int32_t, DBUS_TYPE_BOOLEAN>::iter_get(iter);
        }
    };

    template<> struct converter<std::string>: converter<const char*> {
        static void iter_append(DBusMessageIter* iter, const std::string& t) {
            converter<const char*>::iter_append(iter, t.c_str());
        }
        static std::string iter_get(DBusMessageIter* iter) {
            return converter<const char*>::iter_get(iter);
        }
    };

    template<> struct converter<AString>: converter<std::string> {
        static void iter_append(DBusMessageIter* iter, const AString& t) {
            converter<std::string>::iter_append(iter, t.toStdString());
        }
        static AString iter_get(DBusMessageIter* iter) {
            return converter<const char*>::iter_get(iter);
        }
    };
    template<> struct converter<ObjectPath>: impl::basic_converter<const char*, DBUS_TYPE_OBJECT_PATH> {
        using super = impl::basic_converter<const char*, DBUS_TYPE_OBJECT_PATH>;
        static void iter_append(DBusMessageIter* iter, const ObjectPath& t) {
            super::iter_append(iter, t.c_str());
        }

        static ObjectPath iter_get(DBusMessageIter* iter) {
            return super::iter_get(iter);
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
        static AVector<T> iter_get(DBusMessageIter* iter) {
            if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_ARRAY) {
                throw AException("type error: array expected, got '{:c}'"_format(got));
            }
            DBusMessageIter sub;
            dbus_message_iter_recurse(iter, &sub);

            AVector<T> result;
            for (;;) {
                result << aui::dbus::iter_get<T>(&sub);
                if (!dbus_message_iter_next(&sub)) break;
            }

            return result;
        }
    };

    template<convertible K, convertible V>
    struct converter<AMap<K, V>> {
        static inline std::string signature = fmt::format("a{{{}{}}}", converter<K>::signature, converter<V>::signature);

        static void iter_append(DBusMessageIter* iter, const AMap<K, V>& t) {
            DBusMessageIter sub;
            if (!dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, fmt::format("{{{}{}}}", converter<K>::signature, converter<V>::signature).c_str(), &sub)) {
                throw AException("dbus_message_iter_open_container failed");
            }
            ARaiiHelper h = [&] {
                dbus_message_iter_close_container(iter, &sub);
            };

            for (const auto&[k, v] : t) {
                DBusMessageIter item;
                dbus_message_iter_open_container(&sub, DBUS_TYPE_DICT_ENTRY, nullptr, &item);
                ARaiiHelper r = [&] {
                    dbus_message_iter_close_container(&sub, &item);
                };

                aui::dbus::iter_append(&item, k);
                aui::dbus::iter_append(&item, v);
            }
        }
        static AMap<K, V> iter_get(DBusMessageIter* iter) {
            if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_ARRAY) {
                throw AException("type error: array expected, got '{:c}'"_format(got));
            }
            DBusMessageIter sub;
            dbus_message_iter_recurse(iter, &sub);

            AMap<K, V> result;
            do {
                AUI_ASSERT(dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY);
                DBusMessageIter item;
                dbus_message_iter_recurse(&sub, &item);
                auto k = aui::dbus::iter_get<K>(&item);
                if (!dbus_message_iter_next(&item)) {
                    throw AException("bad dict");
                }
                if (k == "current_filter") { // TODO dirty hack of OpenFile
                    continue;
                }
                auto v = aui::dbus::iter_get<V>(&item);
                result[k] = v;
            } while (dbus_message_iter_next(&sub));

            return result;
        }
    };

    template<convertible... Types>
    struct converter<std::tuple<Types...>> {
        static inline std::string signature = "(" + (... + converter<Types>::signature) + ")";

        static void iter_append(DBusMessageIter* iter, const std::tuple<Types...>& t) {
            if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_STRUCT) {
                throw AException("type error: struct expected, got '{:c}'"_format(got));
            }

            DBusMessageIter sub;
            dbus_message_iter_recurse(iter, &sub);

            std::apply([&](const auto&... args){
                (..., aui::dbus::iter_append(&sub, args));
            }, t);
        }

        static std::tuple<Types...> iter_get(DBusMessageIter* iter) {
            if (auto got = dbus_message_iter_get_arg_type(iter); got != DBUS_TYPE_STRUCT) {
                throw AException("type error: struct expected, got '{:c}'"_format(got));
            }

            DBusMessageIter sub;
            dbus_message_iter_recurse(iter, &sub);

            bool hasNext = true;
            return aui::tuple_visitor<std::tuple<Types...>>::for_each_make_tuple([&]<typename T>() {
                if (!hasNext) {
                    throw AException("too few arguments");
                }
                auto v = aui::dbus::iter_get<T>(&sub);
                hasNext = dbus_message_iter_next(&sub);
                return v;
            });
        }
    };

    template<>
    struct API_AUI_VIEWS converter<Variant> {
        static inline std::string signature = "v";

        static void iter_append(DBusMessageIter* iter, const Variant& t);

        static Variant iter_get(DBusMessageIter* iter);
    };

    template<>
    struct API_AUI_VIEWS converter<Unknown> {
        static inline std::string signature = "";

        static void iter_append(DBusMessageIter* iter, const Unknown& t) {
            // TODO
            AUI_ASSERT(0);
        }

        static Unknown iter_get(DBusMessageIter* iter);
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
        auto msg = aui::ptr::manage_unique(dbus_message_new_method_call(bus.toStdString().c_str(),
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

        auto pending = aui::ptr::manage_unique([&] {
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
            if (!dbus_message_iter_init(msg.get(), &dbusArgs)) {
                throw ADBus::Exception(formatError("dbus replied no arguments"));
            }
            return aui::dbus::iter_get<Return>(&dbusArgs);
        }
    }

    template<aui::not_overloaded_lambda Callback>
    std::function<void()> addSignalListener(aui::dbus::ObjectPath object, const AString& interface, const AString& signal, Callback&& callback) {
        return addListener([object = std::move(object),
                     interface = interface.toStdString(),
                     signal = signal.toStdString(),
                     callback = std::forward<Callback>(callback)](DBusMessage* msg) {
            if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected")) {
                return DBUS_HANDLER_RESULT_HANDLED;
            }

            if (!dbus_message_is_signal(msg, interface.c_str(), signal.c_str())) {
                return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
            }

            if (object != std::string_view(dbus_message_get_path(msg))) {
                return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
            }

            using argz = typename aui::lambda_info<Callback>::args;
            if constexpr (std::tuple_size_v<argz> > 0) {
                DBusMessageIter dbusArgs;
                if (!dbus_message_iter_init(msg, &dbusArgs)) {
                    throw ADBus::Exception("dbus replied no arguments");
                }

                bool hasNext = true;
                auto args = aui::tuple_visitor<argz>::for_each_make_tuple([&]<typename T>() {
                    if (!hasNext) {
                        throw ADBus::Exception("too few arguments");
                    }
                    auto v = aui::dbus::iter_get<T>(&dbusArgs);
                    hasNext = dbus_message_iter_next(&dbusArgs);
                    return v;
                });

                std::apply(callback, std::move(args));
            } else {
                callback();
            }


            return DBUS_HANDLER_RESULT_HANDLED;
        });
    }

    void processMessages();

private:
    struct RawMessageListener {
        ADBus* parent;
        using Callback = std::function<DBusHandlerResult(DBusMessage* message)>;
        Callback function;
    };
    std::list<RawMessageListener> mListeners; // guarantees safe pointers to it's elements
    aui::fast_pimpl<DBusError, sizeof(void*) * 3 + 20, alignof(void*)> mError;
    DBusConnection* mConnection = nullptr;
    std::atomic_bool mProcessingScheduled = false;

    ADBus();
    ~ADBus();

    template <aui::invocable Callback>
    void throwExceptionOnError(Callback&& callback);
    std::function<void()> addListener(RawMessageListener::Callback listener);
    static dbus_bool_t addWatch(DBusWatch* watch, void* data);

    static DBusHandlerResult listener(DBusConnection     *connection,
                                      DBusMessage        *message,
                                      void               *user_data) noexcept;
    static void deleter(void* userData) noexcept;
};
