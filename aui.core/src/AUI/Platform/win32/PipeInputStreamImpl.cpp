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