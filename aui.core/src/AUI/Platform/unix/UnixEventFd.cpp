//
// Created by alex2772 on 4/19/22.
//

#include "UnixEventFd.h"
#ifdef __linux
#include <sys/eventfd.h>
#endif

#include <unistd.h>
#include <csignal>
#include <cstdint>


#if AUI_PLATFORM_APPLE

UnixEventFd::UnixEventFd() noexcept
{
    int r = pipe(&mOut);
    assert(r == 0);
}

UnixEventFd::~UnixEventFd() {
    close(mIn);
    close(mOut);
}

void UnixEventFd::set() noexcept {
    std::uint64_t buffer = 1;
    auto r = write(mOut, &buffer, sizeof(buffer));
    assert(r == sizeof(buffer));
}

void UnixEventFd::reset() noexcept {
    std::uint64_t buffer = 0;
    auto r = read(mIn, &buffer, sizeof(buffer));
    assert(r == sizeof(buffer));
}

#else


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

#endif