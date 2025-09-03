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

#include <AUI/Platform/Pipe.h>
#include <AUI/Traits/strings.h>
#include <cassert>
#include "AUI/Common/AException.h"
#include "AUI/Common/AByteBuffer.h"
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

    // the code uses CreateNamedPipe with the FILE_FLAG_OVERLAPPED flag, which enables overlapped I/O operations. This
    // is useful for non-blocking and asynchronous communication patterns, where a process can send data to or receive
    // data from the pipe without blocking its own execution.

    constexpr auto BUFFER_SIZE = 4096;
    auto pipeName = R"(\\.\Pipe\AuiAnonPipe.{}.{})"_format(GetCurrentProcessId(), nextUniqueId());
    auto wPipeName = pipeName.toWideString();
    mOut = CreateNamedPipe(wPipeName.c_str(),
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
    mIn = CreateFile(reinterpret_cast<const wchar_t*>(pipeNameU16.data()),
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

size_t Pipe::read(char *dst, size_t size) {
    AUI_ASSERT(out() != 0);

    DWORD bytesRead;
    if (!ReadFile(out(), dst, size, &bytesRead, nullptr)) {
        throw AIOException("failed to read from pipe");
    }
    return bytesRead;
}

void Pipe::write(const char *src, size_t size) {
    AUI_ASSERT(in() != 0);

    DWORD bytesWritten;
    if (!WriteFile(in(), src, size, &bytesWritten, nullptr)) {
        throw AIOException("failed to write to pipe");
    }
}
