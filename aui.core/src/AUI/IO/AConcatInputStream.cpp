// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by alex2 on 13.11.2020.
//

#include "AConcatInputStream.h"

size_t AConcatInputStream::read(char* dst, size_t size) {
    std::size_t result = 0;
    while (!mInputStreams.empty() && size > 0) {
        size_t r = mInputStreams.first()->read(dst, size);
        if (r == 0) {
            mInputStreams.pop_front();
            continue;
        }
        dst += r;
        size -= r;
        result += r;
    }
    return result;
}
