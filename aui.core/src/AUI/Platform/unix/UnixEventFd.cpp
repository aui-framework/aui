// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    AUI_ASSERT(r == 0);
}

UnixEventFd::~UnixEventFd() {
    close(mIn);
    close(mOut);
}

void UnixEventFd::set() noexcept {
    std::uint64_t buffer = 1;
    auto r = write(mIn, &buffer, sizeof(buffer));
    AUI_ASSERT(r == sizeof(buffer));
}

void UnixEventFd::reset() noexcept {
    std::uint64_t buffer = 0;
    auto r = read(mOut, &buffer, sizeof(buffer));
    AUI_ASSERT(r == sizeof(buffer));
}

#else


UnixEventFd::UnixEventFd() noexcept: mHandle(eventfd(0, EFD_SEMAPHORE)) {
    AUI_ASSERT(mHandle != -1);
}

UnixEventFd::~UnixEventFd() {
    close(mHandle);
}

void UnixEventFd::set() noexcept {
    std::uint64_t buffer = 1;
    auto r = write(mHandle, &buffer, sizeof(buffer));
    AUI_ASSERT(r == sizeof(buffer));
}

void UnixEventFd::reset() noexcept {
    std::uint64_t buffer = 0;
    auto r = read(mHandle, &buffer, sizeof(buffer));
    AUI_ASSERT(r == sizeof(buffer));
}

#endif