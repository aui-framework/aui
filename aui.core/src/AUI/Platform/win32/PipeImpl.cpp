// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
