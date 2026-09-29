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

#if AUI_PLATFORM_LINUX
#include "../linux/ASingleInstanceDBus.h"
#endif

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <fcntl.h>
#include <poll.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static constexpr auto LOG_TAG = "ASingleInstance";

using namespace std::chrono_literals;
using namespace aui::detail::single_instance;

namespace {
APath runtimeDir() {
    if (auto flatpakId = std::getenv("FLATPAK_ID"); flatpakId && *flatpakId) {
        // $XDG_RUNTIME_DIR itself is private to the sandbox instance, but app/$FLATPAK_ID is shared between all
        // instances of the same flatpak.
        if (auto xdg = std::getenv("XDG_RUNTIME_DIR"); xdg && *xdg) {
            return APath(xdg) / "app" / flatpakId;
        }
    }
    if (auto xdg = std::getenv("XDG_RUNTIME_DIR"); xdg && *xdg) {
        return APath(xdg);
    }
    return APath("/tmp");
}

struct Fd : aui::noncopyable {
    int fd = -1;
    explicit Fd(int fd = -1) : fd(fd) {}
    Fd(Fd&& o) noexcept : fd(std::exchange(o.fd, -1)) {}
    Fd& operator=(Fd&& o) noexcept {
        std::swap(fd, o.fd);
        return *this;
    }
    ~Fd() {
        if (fd >= 0) {
            ::close(fd);
        }
    }
};

bool writeAll(int fd, const void* data, size_t size) {
    auto ptr = static_cast<const char*>(data);
    while (size > 0) {
        auto r = ::send(fd, ptr, size, MSG_NOSIGNAL);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        ptr += r;
        size -= size_t(r);
    }
    return true;
}

bool readAll(int fd, void* data, size_t size) {
    auto ptr = static_cast<char*>(data);
    while (size > 0) {
        auto r = ::recv(fd, ptr, size, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (r == 0) {
            return false;
        }
        ptr += r;
        size -= size_t(r);
    }
    return true;
}

sockaddr_un makeAddress(const APath& path) {
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    const auto& bytes = path.toStdString();
    if (bytes.size() >= sizeof(addr.sun_path)) {
        throw AException("ASingleInstance: socket path is too long: {}"_format(path));
    }
    std::memcpy(addr.sun_path, bytes.data(), bytes.size());
    return addr;
}

void setTimeout(int fd, std::chrono::milliseconds timeout) {
    timeval tv {
        .tv_sec = time_t(timeout.count() / 1000),
        .tv_usec = suseconds_t((timeout.count() % 1000) * 1000),
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

/**
 * @return true if activation has been delivered.
 */
bool forwardActivation(const APath& socketPath) {
    const auto payload = serialize(makeCurrentActivation());
    const auto addr = makeAddress(socketPath);

    // the primary instance may have acquired the lock but not started listening yet; retry for a while.
    for (int attempt = 0; attempt < 40; ++attempt) {
        Fd sock(::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0));
        if (sock.fd < 0) {
            return false;
        }
        setTimeout(sock.fd, 5s);
        if (::connect(sock.fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0) {
            std::this_thread::sleep_for(50ms);
            continue;
        }
        uint32_t size = payload.size();
        if (!writeAll(sock.fd, &size, sizeof(size)) || !writeAll(sock.fd, payload.data(), payload.size())) {
            return false;
        }
        char ack = 0;
        return readAll(sock.fd, &ack, 1) && ack == 1;
    }
    return false;
}
}   // namespace

struct ASingleInstance::Impl {
#if AUI_PLATFORM_LINUX
    _unique<aui::detail::single_instance::dbus::Handle> dbusPrimary;
#endif
    Fd lockFd;
    Fd listenFd;
    Fd wakeRead, wakeWrite;
    APath socketPath;
    Callback callback;
    std::thread thread;

    void serve() {
        pollfd fds[] = {
            { .fd = listenFd.fd, .events = POLLIN },
            { .fd = wakeRead.fd, .events = POLLIN },
        };
        for (;;) {
            if (::poll(fds, std::size(fds), -1) < 0) {
                if (errno == EINTR) continue;
                ALogger::err(LOG_TAG) << "poll failed: " << strerror(errno);
                return;
            }
            if (fds[1].revents) {
                return;
            }
            if (!(fds[0].revents & POLLIN)) {
                continue;
            }
            Fd client(::accept4(listenFd.fd, nullptr, nullptr, SOCK_CLOEXEC));
            if (client.fd < 0) {
                continue;
            }
            setTimeout(client.fd, 5s);
            try {
                uint32_t size = 0;
                if (!readAll(client.fd, &size, sizeof(size)) || size > 16 * 1024 * 1024) {
                    continue;
                }
                std::string payload(size, '\0');
                if (!readAll(client.fd, payload.data(), size)) {
                    continue;
                }
                auto activation = deserialize(payload);
                char ack = 1;
                writeAll(client.fd, &ack, 1);
                if (callback) {
                    callback(std::move(activation));
                }
            } catch (const AException& e) {
                ALogger::warn(LOG_TAG) << "Malformed activation message: " << e;
            }
        }
    }

    ~Impl() {
        if (wakeWrite.fd >= 0) {
            char c = 0;
            [[maybe_unused]] auto unused = ::write(wakeWrite.fd, &c, 1);
        }
        if (thread.joinable()) {
            thread.join();
        }
        if (listenFd.fd >= 0) {
            ::unlink(socketPath.toStdString().c_str());
        }
        // lockFd is closed afterwards, this releases the flock. The lock file itself is intentionally NOT removed:
        // unlinking a lock file is racy (another process may have opened the old inode and a third one may create a
        // new file, resulting in two primary instances). $XDG_RUNTIME_DIR is a tmpfs cleaned up on logout anyway.
    }
};

_unique<ASingleInstance> ASingleInstance::acquire(const AString& key, Callback onActivation) {
    const auto name = "aui-{}-{}"_format(sanitizeKey(key), getuid());
    const auto dir = runtimeDir();
    const auto lockPath = dir / (name + ".lock");
    auto impl = _new<Impl>();
    impl->socketPath = dir / (name + ".sock");

#if AUI_PLATFORM_LINUX
    // prefer org.freedesktop.Application over the session bus: works across sandboxes (Flatpak, Snap) and is
    // understood by desktop environments. Fall back to flock + unix socket when the bus (or GIO) is unavailable.
    {
        auto outcome = aui::detail::single_instance::dbus::tryAcquire(key, onActivation);
        if (outcome.available) {
            if (!outcome.primary) {
                return nullptr;
            }
            impl->dbusPrimary = std::move(outcome.primary);
            return std::make_unique<ASingleInstance>(std::move(impl));
        }
    }
#endif
    impl->callback = std::move(onActivation);

    impl->lockFd = Fd(::open(lockPath.toStdString().c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0600));
    if (impl->lockFd.fd < 0) {
        ALogger::warn(LOG_TAG) << "Can't open lock file " << lockPath << ": " << strerror(errno)
                               << "; assuming primary instance";
        return std::make_unique<ASingleInstance>(std::move(impl));
    }

    // flock is released by the kernel when the process dies, so stale locks are not possible.
    if (::flock(impl->lockFd.fd, LOCK_EX | LOCK_NB) != 0) {
        if (errno != EWOULDBLOCK) {
            ALogger::warn(LOG_TAG) << "flock failed: " << strerror(errno) << "; assuming primary instance";
            return std::make_unique<ASingleInstance>(std::move(impl));
        }
        if (forwardActivation(impl->socketPath)) {
            ALogger::info(LOG_TAG) << "Activation forwarded to the primary instance";
        } else {
            ALogger::warn(LOG_TAG) << "Primary instance is running but did not accept activation";
        }
        return nullptr;
    }

    // we are the primary instance. stale socket (if any) belongs to a dead process since we own the lock.
    ::unlink(impl->socketPath.toStdString().c_str());
    impl->listenFd = Fd(::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0));
    const auto addr = makeAddress(impl->socketPath);
    if (impl->listenFd.fd < 0 ||
        ::bind(impl->listenFd.fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0 ||
        ::listen(impl->listenFd.fd, 8) != 0) {
        ALogger::warn(LOG_TAG) << "Can't listen on " << impl->socketPath << ": " << strerror(errno)
                               << "; other instances won't be able to activate this one";
        impl->listenFd = Fd();
        return std::make_unique<ASingleInstance>(std::move(impl));
    }
    ::chmod(impl->socketPath.toStdString().c_str(), 0600);

    int wake[2];
    if (::pipe(wake) != 0) {
        throw AException("ASingleInstance: pipe failed");
    }
    impl->wakeRead = Fd(wake[0]);
    impl->wakeWrite = Fd(wake[1]);
    ::fcntl(wake[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(wake[1], F_SETFD, FD_CLOEXEC);
    impl->thread = std::thread([raw = impl.get()] { raw->serve(); });
    return std::make_unique<ASingleInstance>(std::move(impl));
}

ASingleInstance::~ASingleInstance() = default;
