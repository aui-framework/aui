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
