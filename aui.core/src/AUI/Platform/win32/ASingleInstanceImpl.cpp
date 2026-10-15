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

#include <AUI/Platform/ASingleInstance.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>

#include <atomic>
#include <thread>
#include <windows.h>

static constexpr auto LOG_TAG = "ASingleInstance";

using namespace std::chrono_literals;
using namespace aui::detail::single_instance;

namespace {
struct Handle : aui::noncopyable {
    HANDLE h = nullptr;
    explicit Handle(HANDLE h = nullptr) : h(h == INVALID_HANDLE_VALUE ? nullptr : h) {}
    Handle(Handle&& o) noexcept : h(std::exchange(o.h, nullptr)) {}
    Handle& operator=(Handle&& o) noexcept {
        std::swap(h, o.h);
        return *this;
    }
    ~Handle() {
        if (h) CloseHandle(h);
    }
};

bool writeAll(HANDLE h, const void* data, DWORD size) {
    auto ptr = static_cast<const char*>(data);
    while (size > 0) {
        DWORD written = 0;
        if (!WriteFile(h, ptr, size, &written, nullptr)) return false;
        ptr += written;
        size -= written;
    }
    return true;
}

bool readAll(HANDLE h, void* data, DWORD size) {
    auto ptr = static_cast<char*>(data);
    while (size > 0) {
        DWORD read = 0;
        if (!ReadFile(h, ptr, size, &read, nullptr) || read == 0) return false;
        ptr += read;
        size -= read;
    }
    return true;
}

std::wstring toWide(const AString& s) {
    auto u16 = s.toUtf16();
    return { u16.begin(), u16.end() };
}

bool forwardActivation(const std::wstring& pipeName) {
    const auto payload = serialize(makeCurrentActivation());

    // let the primary instance bring its window to foreground (we are the process the user just launched, so we are
    // allowed to do so).
    AllowSetForegroundWindow(ASFW_ANY);

    for (int attempt = 0; attempt < 40; ++attempt) {
        Handle pipe(CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
        if (!pipe.h) {
            if (GetLastError() == ERROR_PIPE_BUSY) {
                WaitNamedPipeW(pipeName.c_str(), 100);
            } else {
                std::this_thread::sleep_for(50ms);
            }
            continue;
        }
        uint32_t size = payload.size();
        if (!writeAll(pipe.h, &size, sizeof(size)) || !writeAll(pipe.h, payload.data(), payload.size())) {
            return false;
        }
        char ack = 0;
        return readAll(pipe.h, &ack, 1) && ack == 1;
    }
    return false;
}
}   // namespace

struct ASingleInstance::Impl {
    Handle mutex;
    std::wstring pipeName;
    Callback callback;
    std::atomic_bool stop = false;
    std::thread thread;

    void serve() {
        while (!stop) {
            Handle pipe(CreateNamedPipeW(
                pipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                PIPE_REJECT_REMOTE_CLIENTS, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, nullptr));
            if (!pipe.h) {
                ALogger::err(LOG_TAG) << "CreateNamedPipe failed: " << GetLastError();
                return;
            }
            if (!ConnectNamedPipe(pipe.h, nullptr) && GetLastError() != ERROR_PIPE_CONNECTED) {
                continue;
            }
            if (stop) {
                return;
            }
            try {
                uint32_t size = 0;
                if (!readAll(pipe.h, &size, sizeof(size)) || size > 16 * 1024 * 1024) continue;
                std::string payload(size, '\0');
                if (!readAll(pipe.h, payload.data(), size)) continue;
                auto activation = deserialize(payload);
                char ack = 1;
                writeAll(pipe.h, &ack, 1);
                FlushFileBuffers(pipe.h);
                if (callback) {
                    callback(std::move(activation));
                }
            } catch (const AException& e) {
                ALogger::warn(LOG_TAG) << "Malformed activation message: " << e;
            }
            DisconnectNamedPipe(pipe.h);
        }
    }

    ~Impl() {
        if (thread.joinable()) {
            stop = true;
            // unblock ConnectNamedPipe
            Handle self(CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
            thread.join();
        }
    }
};

_unique<ASingleInstance> ASingleInstance::acquire(const AString& key, Callback onActivation) {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    const auto name = "aui.{}.{}"_format(sanitizeKey(key), sessionId);

    auto impl = _new<Impl>();
    impl->pipeName = toWide("\\\\.\\pipe\\" + name);
    impl->callback = std::move(onActivation);

    // the mutex is released by the OS if the process dies.
    impl->mutex = Handle(CreateMutexW(nullptr, TRUE, toWide("Local\\" + name).c_str()));
    if (!impl->mutex.h) {
        ALogger::warn(LOG_TAG) << "CreateMutex failed: " << GetLastError() << "; assuming primary instance";
        return std::make_unique<ASingleInstance>(std::move(impl));
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (forwardActivation(impl->pipeName)) {
            ALogger::info(LOG_TAG) << "Activation forwarded to the primary instance";
        } else {
            ALogger::warn(LOG_TAG) << "Primary instance is running but did not accept activation";
        }
        return nullptr;
    }

    impl->thread = std::thread([raw = impl.get()] { raw->serve(); });
    return std::make_unique<ASingleInstance>(std::move(impl));
}

ASingleInstance::~ASingleInstance() = default;
