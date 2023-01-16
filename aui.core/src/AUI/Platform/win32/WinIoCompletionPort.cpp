// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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
// Created by Alex2772 on 4/18/2022.
//

#include "WinIoCompletionPort.h"
#include "AUI/Common/AException.h"
#include <atomic>

void WinIoCompletionPort::create(HANDLE fileHandle) {
    mKey = nextKey();
    if (!(mCompletionPortHandle = CreateIoCompletionPort(fileHandle, nullptr, mKey, 0))) {
        throw AException("CreateIoCompletionPort failed");
    }
}

WinIoCompletionPort::~WinIoCompletionPort() {
    if (mCompletionPortHandle) {
        CloseHandle(mCompletionPortHandle);
    }
}

ULONG_PTR WinIoCompletionPort::nextKey() noexcept {
    static std::atomic_uintptr_t key = 0;
    return key++;
}
