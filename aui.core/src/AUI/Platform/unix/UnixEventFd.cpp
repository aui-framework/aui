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

UnixEventFd::UnixEventFd() noexcept
#if !AUI_PLATFORM_APPLE
    : mHandle(eventfd(0, EFD_SEMAPHORE))
#endif
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
