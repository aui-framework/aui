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


PipeOutputStream::PipeOutputStream(Pipe pipe) : mPipe(std::move(pipe)) {
    mFileHandle = fdopen(mPipe.in(),"w");
    AUI_ASSERTX(mFileHandle != nullptr, "invalid pipe");
}
PipeOutputStream::~PipeOutputStream() {
    fclose(mFileHandle);
}

void PipeOutputStream::write(const char* src, size_t size) {
    auto o = fwrite(src, 1, size, mFileHandle);
    AUI_ASSERT(o == size);
}
