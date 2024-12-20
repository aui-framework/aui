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
