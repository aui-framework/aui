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

#include <AUI/Platform/PipeOutputStream.h>


PipeOutputStream::PipeOutputStream(Pipe pipe) : mPipe(std::move(pipe)) {}
PipeOutputStream::~PipeOutputStream() = default;

void PipeOutputStream::write(const char* src, size_t size) {
    DWORD bytesWritten;
    if (!WriteFile(mPipe.in(), src, size, &bytesWritten, nullptr)) {
        throw AIOException("failed to write to pipe");
    }
    AUI_ASSERT(bytesWritten == size);
}
