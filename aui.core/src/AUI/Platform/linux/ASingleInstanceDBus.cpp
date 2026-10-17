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

#include <cstdlib>
#include <cstring>
#include <future>
#include <string>
#include <thread>
#include <vector>
#include <dlfcn.h>

// Single instance on top of GApplication (libgio). GApplication does all the D-Bus work: name ownership, the
// org.freedesktop.Application interface, forwarding of the command line/working directory/environment to the primary
// instance. GIO is loaded dynamically (no glib headers required, aui.core does not link to it); only GApplication is
// used, GTK is not involved, so CLI applications work as well.
namespace gio_fake {
struct GApplication;
struct GApplicationCommandLine;
struct GMainContext;
struct GMainLoop;
struct GError {
    uint32_t domain;
    int code;
    char* message;
};

struct Gio {
    GApplication* (*g_application_new)(const char* id, unsigned flags);
    int (*g_application_id_is_valid)(const char* id);
    int (*g_application_register)(GApplication*, void* cancellable, GError** error);
    int (*g_application_get_is_remote)(GApplication*);
    void* (*g_application_get_dbus_connection)(GApplication*);
    int (*g_application_run)(GApplication*, int argc, char** argv);
    const char* (*g_application_command_line_get_cwd)(GApplicationCommandLine*);
    const char* (*g_application_command_line_getenv)(GApplicationCommandLine*, const char* name);
    char** (*g_application_command_line_get_arguments)(GApplicationCommandLine*, int* argc);
    unsigned long (*g_signal_connect_data)(void* instance, const char* signal, void (*handler)(), void* data,
                                           void* destroy, int flags);
    void (*g_object_unref)(void*);
    void (*g_strfreev)(char**);
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
        AUI_GIO_SYM(g_application_new)
        AUI_GIO_SYM(g_application_id_is_valid)
        AUI_GIO_SYM(g_application_register)
        AUI_GIO_SYM(g_application_get_is_remote)
        AUI_GIO_SYM(g_application_get_dbus_connection)
        AUI_GIO_SYM(g_application_run)
        AUI_GIO_SYM(g_application_command_line_get_cwd)
        AUI_GIO_SYM(g_application_command_line_getenv)
        AUI_GIO_SYM(g_application_command_line_get_arguments)
        AUI_GIO_SYM(g_signal_connect_data)
        AUI_GIO_SYM(g_object_unref)
        AUI_GIO_SYM(g_strfreev)
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
using namespace aui::detail::single_instance;
using aui::detail::single_instance::dbus::Handle;

static constexpr auto LOG_TAG = "ASingleInstance/GApplication";

namespace {

constexpr unsigned G_APPLICATION_HANDLES_COMMAND_LINE = 1 << 3;
constexpr unsigned G_APPLICATION_SEND_ENVIRONMENT = 1 << 4;

struct Primary : Handle {
    enum class Status { Unavailable, Primary, Forwarded };

    const Gio& gio;
    ASingleInstance::Callback callback;
    GMainContext* context = nullptr;
    GMainLoop* loop = nullptr;
    std::thread thread;

    explicit Primary(const Gio& gio) : gio(gio) {}

    ~Primary() override {
        if (thread.joinable()) {
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

    static int onCommandLine(GApplication*, GApplicationCommandLine* cmd, void* userData) {
        auto self = static_cast<Primary*>(userData);
        const auto& gio = self->gio;
        try {
            AActivation activation;
            int argc = 0;
            char** argv = gio.g_application_command_line_get_arguments(cmd, &argc);
            for (int i = 0; i < argc; ++i) {
                activation.args << AString::fromUtf8(argv[i]);
            }
            gio.g_strfreev(argv);
            if (auto cwd = gio.g_application_command_line_get_cwd(cmd)) {
                activation.workingDir = APath(AString::fromUtf8(cwd));
            }
            for (const char* env : { "XDG_ACTIVATION_TOKEN", "DESKTOP_STARTUP_ID" }) {
                if (auto v = gio.g_application_command_line_getenv(cmd, env); v && *v) {
                    activation.activationToken = AString::fromUtf8(v);
                    break;
                }
            }
            if (self->callback) {
                self->callback(std::move(activation));
            }
        } catch (const std::exception& e) {
            // exceptions must never propagate through C frames of GLib
            ALogger::warn(LOG_TAG) << "Failed to handle activation: " << e.what();
            return 1;
        }
        return 0;
    }

    void run(const std::string id, std::promise<Status>& ready) {
        gio.g_main_context_push_thread_default(context);

        auto app = gio.g_application_new(id.c_str(), G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_SEND_ENVIRONMENT);
        gio.g_signal_connect_data(app, "command-line", reinterpret_cast<void (*)()>(&Primary::onCommandLine), this,
                                  nullptr, 0);

        GError* error = nullptr;
        if (!gio.g_application_register(app, nullptr, &error)) {
            ALogger::info(LOG_TAG) << "can't register: " << (error ? error->message : "unknown error");
            if (error) {
                gio.g_error_free(error);
            }
            gio.g_main_context_pop_thread_default(context);
            gio.g_object_unref(app);
            ready.set_value(Status::Unavailable);
            return;
        }

        if (!gio.g_application_get_dbus_connection(app)) {
            // without the session bus GApplication silently degrades to a local non-unique application.
            ALogger::info(LOG_TAG) << "session bus is not available";
            gio.g_main_context_pop_thread_default(context);
            gio.g_object_unref(app);
            ready.set_value(Status::Unavailable);
            return;
        }

        if (gio.g_application_get_is_remote(app)) {
            // another process owns the name. g_application_run() forwards our command line, working directory and
            // environment to it and returns without running a main loop.
            gio.g_main_context_pop_thread_default(context);
            std::vector<std::string> storage;
            for (const auto& a : makeCurrentActivation().args) {
                storage.push_back(a.toStdString());
            }
            std::vector<char*> argv;
            for (auto& a : storage) {
                argv.push_back(a.data());
            }
            argv.push_back(nullptr);
            auto exitCode = gio.g_application_run(app, int(storage.size()), argv.data());
            gio.g_object_unref(app);
            if (exitCode != 0) {
                ALogger::warn(LOG_TAG) << "Primary instance rejected activation (" << exitCode << ")";
            }
            ready.set_value(Status::Forwarded);
            return;
        }

        ready.set_value(Status::Primary);
        gio.g_main_loop_run(loop);

        gio.g_main_context_pop_thread_default(context);
        gio.g_object_unref(app);   // releases the bus name
    }
};
}   // namespace

bool aui::detail::single_instance::dbus::isValidBusName(const AString& key) {
    const auto gio = Gio::get();
    if (!gio) {
        return false;
    }
    return gio->g_application_id_is_valid(key.toStdString().c_str());
}

aui::detail::single_instance::dbus::Outcome aui::detail::single_instance::dbus::tryAcquire(
    const AString& key, ASingleInstance::Callback callback) {
    if (auto forced = std::getenv("AUI_SINGLE_INSTANCE_BACKEND"); forced && std::strcmp(forced, "gapplication") != 0) {
        return {};
    }
    const Gio* gio = Gio::get();
    if (!gio || !isValidBusName(key)) {
        return {};
    }

    auto primary = std::make_unique<Primary>(*gio);
    primary->callback = std::move(callback);
    primary->context = gio->g_main_context_new();
    primary->loop = gio->g_main_loop_new(primary->context, 0);

    std::promise<Primary::Status> ready;
    auto future = ready.get_future();
    primary->thread = std::thread([raw = primary.get(), id = key.toStdString(), &ready] { raw->run(id, ready); });
    const auto status = future.get();

    if (status == Primary::Status::Primary) {
        ALogger::info(LOG_TAG) << "Primary instance (" << key << ")";
        return { .available = true, .primary = std::move(primary) };
    }
    primary->thread.join();   // ~Primary releases loop and context
    if (status == Primary::Status::Unavailable) {
        return {};
    }
    ALogger::info(LOG_TAG) << "Activation forwarded to the primary instance";
    return { .available = true };
}
