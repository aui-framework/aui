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
