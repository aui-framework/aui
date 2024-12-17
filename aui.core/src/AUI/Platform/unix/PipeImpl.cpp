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
