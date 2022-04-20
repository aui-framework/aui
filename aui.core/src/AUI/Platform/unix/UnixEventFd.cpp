//
// Created by alex2772 on 4/19/22.
//

#include "UnixEventFd.h"
#include <sys/eventfd.h>
#include <csignal>
#include <cstdint>

UnixEventFd::UnixEventFd() noexcept: mHandle(eventfd(0, EFD_SEMAPHORE)) {
    assert(mHandle != -1);
}

UnixEventFd::~UnixEventFd() {
    close(mHandle);
}

void UnixEventFd::set() noexcept {
    std::uint64_t buffer = 1;
    auto r = write(mHandle, &buffer, sizeof(buffer));
    assert(r == sizeof(buffer));
}

void UnixEventFd::reset() noexcept {
    std::uint64_t buffer = 0;
    auto r = read(mHandle, &buffer, sizeof(buffer));
    assert(r == sizeof(buffer));
}
