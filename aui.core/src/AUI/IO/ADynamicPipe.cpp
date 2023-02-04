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

//
// Created by Alex2772 on 9/7/2022.
//

#include "ADynamicPipe.h"

size_t ADynamicPipe::read(char* dst, size_t size) {
    std::size_t result = 0;

    while (size > 0) {
        if (mQueue.empty()) return result;
        auto& f = mQueue.front();
        auto dataToRead = glm::min(f.readerRemaining(), size);
        std::memcpy(dst, f.data.data() + f.readerPos, dataToRead);
        f.readerPos += dataToRead;
        dst += dataToRead;
        size -= dataToRead;
        result += dataToRead;

        if (f.readerRemaining() == 0) {
            mQueue.pop();
        }
    }
    return result;
}

void ADynamicPipe::write(const char* src, size_t size) {
    // check for the space in the last fragment
    if (!mQueue.empty()) {
        mQueue.back().push(src, size);
    }

    // write remaining data
    while (size > 0) {
        mQueue.push({});
        auto& fragment = mQueue.back();
        fragment.push(src, size);
    }
}
