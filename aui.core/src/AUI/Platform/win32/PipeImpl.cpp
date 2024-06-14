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

#include <AUI/Platform/Pipe.h>
#include <AUI/Traits/strings.h>
#include <cassert>
#include "AUI/Common/AException.h"
#include <atomic>

static std::uintptr_t nextUniqueId() noexcept {
    static std::atomic_uintptr_t id = 0;
    return id++;
}

Pipe::Pipe() {

    SECURITY_ATTRIBUTES securityAttributes;
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = true;
    securityAttributes.lpSecurityDescriptor = nullptr;

    constexpr auto BUFFER_SIZE = 4096;
    auto pipeName = R"(\\.\Pipe\AuiAnonPipe.{}.{})"_format(GetCurrentProcessId(), nextUniqueId());
    mOut = CreateNamedPipe(pipeName.c_str(),
                           PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                           PIPE_TYPE_BYTE | PIPE_WAIT,
                           1,
                           BUFFER_SIZE,
                           BUFFER_SIZE,
                           60 * 1000, // 1min
                           &securityAttributes);

    if (!mOut) {
        throw AException("CreateNamedPipe failed");
    }
    mIn = CreateFile(pipeName.c_str(),
                     GENERIC_WRITE | FILE_FLAG_OVERLAPPED,
                     false, // no sharing
                     &securityAttributes,
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL,
                     nullptr);
    if (mIn == INVALID_HANDLE_VALUE) {
        CloseHandle(mOut);
        throw AException("Pipe CreateFile failed");
    }
}

Pipe::~Pipe() {
    closeIn();
    closeOut();
}

void Pipe::closeIn() noexcept {
    if (mIn) {
        CloseHandle(mIn);
        mIn = nullptr;
    }
}
void Pipe::closeOut() noexcept {
    if (mOut) {
        CloseHandle(mOut);
        mOut = nullptr;
    }
}
