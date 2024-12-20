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
// Created by alex2 on 16.11.2020.
//

#include "ALimitedInputStream.h"

size_t ALimitedInputStream::read(char* dst, size_t size) {
    size_t toRead = glm::min(mLimit, size);
    if (toRead) {
        mLimit -= toRead;
        size_t read = mInputStream->read(dst, toRead);
        AUI_ASSERT(read <= toRead);
        return read;
    }
    return 0;
}
