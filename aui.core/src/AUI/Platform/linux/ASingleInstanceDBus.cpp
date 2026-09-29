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

#include "ASingleInstanceDBus.h"

#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <future>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <dlfcn.h>

// GIO is loaded dynamically and its ABI is stable, so glib headers are not required to build AUI. Types below are
// deliberately opaque/minimal and live in their own namespace to avoid clashes with the real headers.
namespace gio_fake {
struct GVariant;
struct GVariantType;
struct GVariantBuilder;
struct GDBusConnection;
struct GDBusNodeInfo;
struct GDBusInterfaceInfo;
struct GDBusMethodInvocation;
struct GMainContext;
struct GMainLoop;
struct GError {
    uint32_t domain;
    int code;
    char* message;
};

struct GDBusInterfaceVTable {
    void (*method_call)(GDBusConnection*, const char* sender, const char* objectPath, const char* interfaceName,
                        const char* methodName, GVariant* parameters, GDBusMethodInvocation* invocation, void* userData);
    void* get_property;
    void* set_property;
    void* padding[8];
};

struct Gio {
    GDBusConnection* (*g_bus_get_sync)(int busType, void* cancellable, GError** error);
    GVariant* (*g_dbus_connection_call_sync)(GDBusConnection*, const char* busName, const char* objectPath,
                                             const char* interfaceName, const char* methodName, GVariant* parameters,
                                             const GVariantType* replyType, int flags, int timeoutMsec,
                                             void* cancellable, GError** error);
    unsigned (*g_dbus_connection_register_object)(GDBusConnection*, const char* objectPath, GDBusInterfaceInfo*,
                                                  const GDBusInterfaceVTable*, void* userData, void* destroy,
                                                  GError** error);
    int (*g_dbus_connection_unregister_object)(GDBusConnection*, unsigned id);
    GDBusNodeInfo* (*g_dbus_node_info_new_for_xml)(const char* xml, GError** error);
    GDBusInterfaceInfo* (*g_dbus_node_info_lookup_interface)(GDBusNodeInfo*, const char* name);
    void (*g_dbus_node_info_unref)(GDBusNodeInfo*);
    void (*g_dbus_method_invocation_return_value)(GDBusMethodInvocation*, GVariant*);
    void (*g_dbus_method_invocation_return_dbus_error)(GDBusMethodInvocation*, const char* name, const char* message);

    GVariant* (*g_variant_new)(const char* format, ...);
    GVariant* (*g_variant_new_string)(const char*);
    GVariant* (*g_variant_new_strv)(const char* const*, ssize_t length);
    GVariant* (*g_variant_new_variant)(GVariant*);
    GVariantType* (*g_variant_type_new)(const char*);
    void (*g_variant_type_free)(GVariantType*);
    GVariantBuilder* (*g_variant_builder_new)(const GVariantType*);
    void (*g_variant_builder_add)(GVariantBuilder*, const char* format, ...);
    GVariant* (*g_variant_builder_end)(GVariantBuilder*);
    void (*g_variant_builder_unref)(GVariantBuilder*);
    void (*g_variant_unref)(GVariant*);
    GVariant* (*g_variant_get_child_value)(GVariant*, size_t index);
    size_t (*g_variant_n_children)(GVariant*);
    const char* (*g_variant_get_string)(GVariant*, size_t* length);
    const char** (*g_variant_get_strv)(GVariant*, size_t* length);
    uint32_t (*g_variant_get_uint32)(GVariant*);
    GVariant* (*g_variant_lookup_value)(GVariant* dictionary, const char* key, const GVariantType* expectedType);
    void (*g_free)(void*);
    void (*g_error_free)(GError*);

    GMainContext* (*g_main_context_new)();
    void (*g_main_context_unref)(GMainContext*);
    void (*g_main_context_push_thread_default)(GMainContext*);
    void (*g_main_context_pop_thread_default)(GMainContext*);
    int (*g_main_context_invoke)(GMainContext*, int (*function)(void*), void* data);
    GMainLoop* (*g_main_loop_new)(GMainContext*, int isRunning);
    void (*g_main_loop_run)(GMainLoop*);
    void (*g_main_loop_quit)(GMainLoop*);
    void (*g_main_loop_unref)(GMainLoop*);

    static const Gio* get() {
        static const Gio* instance = load();
        return instance;
    }

private:
    static const Gio* load() {
        void* handle = dlopen("libgio-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
        if (!handle) {
            return nullptr;
        }
        static Gio result {};
#define AUI_GIO_SYM(name)                                                                                              \
    result.name = reinterpret_cast<decltype(result.name)>(dlsym(handle, #name));                                       \
    if (!result.name) {                                                                                                \
        return nullptr;                                                                                                \
    }
        AUI_GIO_SYM(g_bus_get_sync)
        AUI_GIO_SYM(g_dbus_connection_call_sync)
        AUI_GIO_SYM(g_dbus_connection_register_object)
        AUI_GIO_SYM(g_dbus_connection_unregister_object)
        AUI_GIO_SYM(g_dbus_node_info_new_for_xml)
        AUI_GIO_SYM(g_dbus_node_info_lookup_interface)
        AUI_GIO_SYM(g_dbus_node_info_unref)
        AUI_GIO_SYM(g_dbus_method_invocation_return_value)
        AUI_GIO_SYM(g_dbus_method_invocation_return_dbus_error)
        AUI_GIO_SYM(g_variant_new)
        AUI_GIO_SYM(g_variant_new_string)
        AUI_GIO_SYM(g_variant_new_strv)
        AUI_GIO_SYM(g_variant_new_variant)
        AUI_GIO_SYM(g_variant_type_new)
        AUI_GIO_SYM(g_variant_type_free)
        AUI_GIO_SYM(g_variant_builder_new)
        AUI_GIO_SYM(g_variant_builder_add)
        AUI_GIO_SYM(g_variant_builder_end)
        AUI_GIO_SYM(g_variant_builder_unref)
        AUI_GIO_SYM(g_variant_unref)
        AUI_GIO_SYM(g_variant_get_child_value)
        AUI_GIO_SYM(g_variant_n_children)
        AUI_GIO_SYM(g_variant_get_string)
        AUI_GIO_SYM(g_variant_get_strv)
        AUI_GIO_SYM(g_variant_get_uint32)
        AUI_GIO_SYM(g_variant_lookup_value)
        AUI_GIO_SYM(g_free)
        AUI_GIO_SYM(g_error_free)
        AUI_GIO_SYM(g_main_context_new)
        AUI_GIO_SYM(g_main_context_unref)
        AUI_GIO_SYM(g_main_context_push_thread_default)
        AUI_GIO_SYM(g_main_context_pop_thread_default)
        AUI_GIO_SYM(g_main_context_invoke)
        AUI_GIO_SYM(g_main_loop_new)
        AUI_GIO_SYM(g_main_loop_run)
        AUI_GIO_SYM(g_main_loop_quit)
        AUI_GIO_SYM(g_main_loop_unref)
#undef AUI_GIO_SYM
        return &result;
    }
};
}   // namespace gio_fake

using namespace gio_fake;
using namespace std::chrono_literals;
using namespace aui::detail::single_instance;
using aui::detail::single_instance::dbus::Handle;

static constexpr auto LOG_TAG = "ASingleInstance/DBus";

namespace {

constexpr int G_BUS_TYPE_SESSION = 2;
constexpr uint32_t DBUS_NAME_FLAG_DO_NOT_QUEUE = 4;
constexpr uint32_t DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER = 1;
constexpr int CALL_TIMEOUT_MS = 5000;

constexpr auto INTERFACE_XML = R"(<node>
  <interface name='org.freedesktop.Application'>
    <method name='Activate'>
      <arg type='a{sv}' name='platform_data' direction='in'/>
    </method>
    <method name='Open'>
      <arg type='as' name='uris' direction='in'/>
      <arg type='a{sv}' name='platform_data' direction='in'/>
    </method>
    <method name='ActivateAction'>
      <arg type='s' name='action_name' direction='in'/>
      <arg type='av' name='parameter' direction='in'/>
      <arg type='a{sv}' name='platform_data' direction='in'/>
    </method>
  </interface>
</node>)";

// AUI extension of platform_data: the whole command line and working directory of the secondary instance.
constexpr auto KEY_ARGS = "aui-args";
constexpr auto KEY_CWD = "aui-cwd";
// freedesktop.org Application spec
constexpr auto KEY_TOKEN = "activation-token";
constexpr auto KEY_STARTUP_ID = "desktop-startup-id";

std::string objectPathFor(const AString& key) {
    std::string result = "/";
    for (char c : key.toStdString()) {
        result += c == '.' ? '/' : (c == '-' ? '_' : c);
    }
    return result;
}

// g_bus_get_sync returns a connection shared within the process, so the same object path can't be exported twice.
// Names owned by this process are tracked to treat a repeated acquire like a secondary instance.
std::mutex ownedNamesMutex;
std::set<std::string> ownedNames;

struct Err {
    GError* error = nullptr;
    const Gio& gio;
    explicit Err(const Gio& gio) : gio(gio) {}
    ~Err() {
        if (error) {
            gio.g_error_free(error);
        }
    }
    GError** operator&() { return &error; }
    std::string message() const { return error && error->message ? error->message : "unknown error"; }
};

struct Variant {
    GVariant* v = nullptr;
    const Gio& gio;
    Variant(const Gio& gio, GVariant* v) : v(v), gio(gio) {}
    Variant(const Variant&) = delete;
    ~Variant() {
        if (v) {
            gio.g_variant_unref(v);
        }
    }
    explicit operator bool() const { return v != nullptr; }
};

struct Type {
    GVariantType* t;
    const Gio& gio;
    Type(const Gio& gio, const char* spec) : t(gio.g_variant_type_new(spec)), gio(gio) {}
    Type(const Type&) = delete;
    ~Type() { gio.g_variant_type_free(t); }
};

/**
 * @return org.freedesktop.DBus.RequestName reply code; nullopt on failure.
 */
std::optional<uint32_t> requestName(const Gio& gio, GDBusConnection* connection, const std::string& name) {
    Err err(gio);
    Variant reply(gio, gio.g_dbus_connection_call_sync(
                           connection, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                           "RequestName", gio.g_variant_new("(su)", name.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE), nullptr,
                           0, CALL_TIMEOUT_MS, nullptr, &err));
    if (!reply) {
        ALogger::info(LOG_TAG) << "RequestName failed: " << err.message();
        return std::nullopt;
    }
    Variant code(gio, gio.g_variant_get_child_value(reply.v, 0));
    return gio.g_variant_get_uint32(code.v);
}

std::string lookupString(const Gio& gio, GVariant* platformData, const char* key) {
    Type type(gio, "s");
    Variant value(gio, gio.g_variant_lookup_value(platformData, key, type.t));
    if (!value) {
        return {};
    }
    return gio.g_variant_get_string(value.v, nullptr);
}

std::vector<std::string> lookupStrv(const Gio& gio, GVariant* platformData, const char* key) {
    Type type(gio, "as");
    Variant value(gio, gio.g_variant_lookup_value(platformData, key, type.t));
    std::vector<std::string> result;
    if (!value) {
        return result;
    }
    size_t n = 0;
    auto strv = gio.g_variant_get_strv(value.v, &n);
    for (size_t i = 0; i < n; ++i) {
        result.emplace_back(strv[i]);
    }
    gio.g_free(strv);
    return result;
}

/**
 * @return true if activation has been delivered.
 */
bool forwardActivation(const Gio& gio, const AString& key) {
    Err err(gio);
    auto connection = gio.g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
    if (!connection) {
        return false;
    }
    const auto activation = makeCurrentActivation();

    Type dictType(gio, "a{sv}");
    auto builder = gio.g_variant_builder_new(dictType.t);
    {
        std::vector<std::string> argsStorage;
        for (const auto& a : activation.args) {
            argsStorage.push_back(a.toStdString());
        }
        std::vector<const char*> argv;
        for (const auto& a : argsStorage) {
            argv.push_back(a.c_str());
        }
        gio.g_variant_builder_add(
            builder, "{sv}", KEY_ARGS, gio.g_variant_new_strv(argv.data(), ssize_t(argv.size())));
        gio.g_variant_builder_add(
            builder, "{sv}", KEY_CWD, gio.g_variant_new_string(activation.workingDir.toStdString().c_str()));
        if (!activation.activationToken.empty()) {
            gio.g_variant_builder_add(
                builder, "{sv}", KEY_TOKEN,
                gio.g_variant_new_string(activation.activationToken.toStdString().c_str()));
            gio.g_variant_builder_add(
                builder, "{sv}", KEY_STARTUP_ID,
                gio.g_variant_new_string(activation.activationToken.toStdString().c_str()));
        }
    }
    auto platformData = gio.g_variant_builder_end(builder);
    gio.g_variant_builder_unref(builder);

    const auto name = key.toStdString();
    Err callErr(gio);
    // g_variant_new consumes the floating reference of platformData.
    Variant reply(gio, gio.g_dbus_connection_call_sync(
                           connection, name.c_str(), objectPathFor(key).c_str(), "org.freedesktop.Application",
                           "Activate", gio.g_variant_new("(@a{sv})", platformData), nullptr, 0, CALL_TIMEOUT_MS,
                           nullptr, &callErr));
    if (!reply) {
        ALogger::warn(LOG_TAG) << "Activate failed: " << callErr.message();
        return false;
    }
    return true;
}

struct Primary : Handle {
    const Gio& gio;
    ASingleInstance::Callback callback;
    GMainContext* context = nullptr;
    GMainLoop* loop = nullptr;
    std::thread thread;

    enum class Status { Unavailable, Primary, NameTaken };

    explicit Primary(const Gio& gio) : gio(gio) {}

    ~Primary() override {
        struct ReleaseOwnedName {
            Primary* self;
            ~ReleaseOwnedName() {
                if (!self->ownedName.empty()) {
                    std::lock_guard lock(ownedNamesMutex);
                    ownedNames.erase(self->ownedName);
                }
            }
        } releaseOwnedName { this };
        if (context && thread.joinable()) {
            gio.g_main_context_invoke(
                context,
                [](void* data) -> int {
                    auto self = static_cast<Primary*>(data);
                    self->gio.g_main_loop_quit(self->loop);
                    return 0;
                },
                this);
            thread.join();
        }
        if (loop) {
            gio.g_main_loop_unref(loop);
        }
        if (context) {
            gio.g_main_context_unref(context);
        }
    }

    static void onMethodCall(GDBusConnection*, const char*, const char*, const char*, const char* methodName,
                             GVariant* parameters, GDBusMethodInvocation* invocation, void* userData) {
        auto self = static_cast<Primary*>(userData);
        const auto& gio = self->gio;
        try {
            std::string method = methodName;
            AActivation activation;
            Variant platformData(gio, nullptr);
            std::vector<std::string> uris;
            if (method == "Activate") {
                platformData.v = gio.g_variant_get_child_value(parameters, 0);
            } else if (method == "Open") {
                Variant uriList(gio, gio.g_variant_get_child_value(parameters, 0));
                size_t n = 0;
                auto strv = gio.g_variant_get_strv(uriList.v, &n);
                for (size_t i = 0; i < n; ++i) {
                    uris.emplace_back(strv[i]);
                }
                gio.g_free(strv);
                platformData.v = gio.g_variant_get_child_value(parameters, 1);
            } else if (method == "ActivateAction") {
                platformData.v = gio.g_variant_get_child_value(parameters, 2);
            } else {
                gio.g_dbus_method_invocation_return_dbus_error(
                    invocation, "org.freedesktop.DBus.Error.UnknownMethod", "unknown method");
                return;
            }

            for (const auto& a : lookupStrv(gio, platformData.v, KEY_ARGS)) {
                activation.args << AString::fromUtf8(a);
            }
            if (activation.args.empty()) {
                // launched by a desktop environment (DBusActivatable) or by a non-AUI client.
                if (auto own = makeCurrentActivation().args; !own.empty()) {
                    activation.args << own.first();
                }
            }
            for (const auto& uri : uris) {
                activation.args << AString::fromUtf8(uri);
            }
            activation.workingDir = APath(AString::fromUtf8(lookupString(gio, platformData.v, KEY_CWD)));
            activation.activationToken = AString::fromUtf8(lookupString(gio, platformData.v, KEY_TOKEN));
            if (activation.activationToken.empty()) {
                activation.activationToken = AString::fromUtf8(lookupString(gio, platformData.v, KEY_STARTUP_ID));
            }

            gio.g_dbus_method_invocation_return_value(invocation, nullptr);
            if (self->callback) {
                self->callback(std::move(activation));
            }
        } catch (const std::exception& e) {
            // exceptions must never propagate through C frames of GLib
            ALogger::warn(LOG_TAG) << "Malformed activation message: " << e.what();
            gio.g_dbus_method_invocation_return_dbus_error(
                invocation, "org.freedesktop.DBus.Error.InvalidArgs", "malformed activation");
        }
    }

    std::string ownedName;

    void run(const std::string name, const std::string path, std::promise<Status>& ready) {
        {
            std::lock_guard lock(ownedNamesMutex);
            if (!ownedNames.insert(name).second) {
                ready.set_value(Status::NameTaken);
                return;
            }
            ownedName = name;
        }
        gio.g_main_context_push_thread_default(context);
        AUI_DEFER { gio.g_main_context_pop_thread_default(context); };

        Err err(gio);
        auto connection = gio.g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
        if (!connection) {
            ALogger::info(LOG_TAG) << "session bus is not available: " << err.message();
            ready.set_value(Status::Unavailable);
            return;
        }
        auto node = gio.g_dbus_node_info_new_for_xml(INTERFACE_XML, &err);
        if (!node) {
            ready.set_value(Status::Unavailable);
            return;
        }
        AUI_DEFER { gio.g_dbus_node_info_unref(node); };
        static const GDBusInterfaceVTable vtable { .method_call = &Primary::onMethodCall };
        auto id = gio.g_dbus_connection_register_object(
            connection, path.c_str(),
            gio.g_dbus_node_info_lookup_interface(node, "org.freedesktop.Application"), &vtable, this, nullptr, &err);
        if (id == 0) {
            ALogger::warn(LOG_TAG) << "can't export object: " << err.message();
            ready.set_value(Status::Unavailable);
            return;
        }
        AUI_DEFER { gio.g_dbus_connection_unregister_object(connection, id); };

        // the object is exported *before* requesting the name so a secondary instance never sees a name without object.
        auto code = requestName(gio, connection, name);
        if (!code) {
            ready.set_value(Status::Unavailable);
            return;
        }
        // ALREADY_OWNER is possible only if the same process (sharing the bus connection) has acquired the name before,
        // i.e., we are effectively a secondary instance.
        if (*code != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
            ready.set_value(Status::NameTaken);
            return;
        }
        ready.set_value(Status::Primary);
        gio.g_main_loop_run(loop);

        Err releaseErr(gio);
        Variant reply(gio, gio.g_dbus_connection_call_sync(
                               connection, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
                               "ReleaseName", gio.g_variant_new("(s)", name.c_str()), nullptr, 0, CALL_TIMEOUT_MS,
                               nullptr, &releaseErr));
        if (!reply) {
            ALogger::warn(LOG_TAG) << "ReleaseName failed: " << releaseErr.message();
        }
    }
};
}   // namespace

bool aui::detail::single_instance::dbus::isValidBusName(const AString& key) {
    const auto s = key.toStdString();
    if (s.empty() || s.size() > 255) {
        return false;
    }
    size_t elements = 0;
    size_t elementLength = 0;
    for (char c : s) {
        if (c == '.') {
            if (elementLength == 0) {
                return false;
            }
            elementLength = 0;
            continue;
        }
        const bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-';
        const bool digit = c >= '0' && c <= '9';
        if (!letter && !digit) {
            return false;
        }
        if (elementLength == 0) {
            if (digit) {
                return false;
            }
            ++elements;
        }
        ++elementLength;
    }
    return elementLength > 0 && elements >= 2;
}

aui::detail::single_instance::dbus::Outcome aui::detail::single_instance::dbus::tryAcquire(
    const AString& key, ASingleInstance::Callback callback) {
    if (auto forced = std::getenv("AUI_SINGLE_INSTANCE_BACKEND"); forced && std::strcmp(forced, "dbus") != 0) {
        return {};
    }
    if (!isValidBusName(key)) {
        return {};
    }
    const Gio* gio = Gio::get();
    if (!gio) {
        return {};
    }

    auto primary = std::make_unique<Primary>(*gio);
    primary->callback = std::move(callback);
    primary->context = gio->g_main_context_new();
    primary->loop = gio->g_main_loop_new(primary->context, 0);

    std::promise<Primary::Status> ready;
    auto future = ready.get_future();
    const auto name = key.toStdString();
    const auto path = objectPathFor(key);
    primary->thread = std::thread([raw = primary.get(), name, path, &ready] {
        raw->run(name, path, ready);
    });
    const auto status = future.get();

    if (status == Primary::Status::Primary) {
        ALogger::info(LOG_TAG) << "Primary instance (D-Bus name " << key << ")";
        return { .available = true, .primary = std::move(primary) };
    }
    primary->thread.join();   // ~Primary releases loop and context

    if (status == Primary::Status::Unavailable) {
        return {};
    }
    if (forwardActivation(*gio, key)) {
        ALogger::info(LOG_TAG) << "Activation forwarded to the primary instance";
    } else {
        ALogger::warn(LOG_TAG) << "Primary instance is running but did not accept activation";
    }
    return { .available = true };
}
