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

#include <AUI/Platform/PipeInputStream.h>

PipeInputStream::PipeInputStream(Pipe pipe) : mPipe(std::move(pipe)) {
    mFileHandle = fdopen(mPipe.out(),"r");
    AUI_ASSERTX(mFileHandle != nullptr, "invalid pipe");
}

PipeInputStream::~PipeInputStream() {
    fclose(mFileHandle);
}

size_t PipeInputStream::read(char* dst, size_t size) {
    return fread(dst, 1, size, mFileHandle);
}

