/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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