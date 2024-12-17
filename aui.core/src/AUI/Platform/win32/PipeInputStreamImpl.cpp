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

#include <AUI/Platform/PipeInputStream.h>

PipeInputStream::PipeInputStream(Pipe pipe) : mPipe(std::move(pipe)) {}
PipeInputStream::~PipeInputStream() = default;

size_t PipeInputStream::read(char* dst, size_t size) {
    DWORD bytesRead;
    if (!ReadFile(mPipe.out(), dst, size, &bytesRead, nullptr)) {
        if (GetLastError() == ERROR_BROKEN_PIPE) {
            return 0;
        }
        throw AIOException("failed to read from pipe");
    }
    return bytesRead;
}