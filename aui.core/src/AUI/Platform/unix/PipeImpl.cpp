// AUI Framework - Declarative UI toolkit for modern C++20
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
