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
#include <cassert>
#include "AUI/Common/AException.h"
#include "AUI/IO/AIOException.h"

Pipe::Pipe() {
    int pipes[2];
    if (pipe(pipes) == -1) {
        throw AException("could not create unix pipe");
    }
    mOut = pipes[0];
    mIn = pipes[1];
}

Pipe::~Pipe() {
    closeIn();
    closeOut();
}

void Pipe::closeIn() noexcept {
    if (mIn) {
        close(mIn);
        mIn = 0;
    }
}
void Pipe::closeOut() noexcept {
    if (mOut) {
        close(mOut);
        mOut = 0;
    }
}

size_t Pipe::read(char *dst, size_t size) {
    AUI_ASSERT(out() != 0);
    return ::read(out(), dst, size);
}

void Pipe::write(const char *src, size_t size) {
    AUI_ASSERT(in() != 0);
    while (size > 0) {
        auto r = ::write(in(), src, size);
        if (r < 0) {
            throw AIOException("write failed");
        }
        src += r;
        size -= r;
    }
}
