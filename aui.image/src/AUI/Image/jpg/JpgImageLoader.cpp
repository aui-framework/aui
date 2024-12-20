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
// Created by alex2 on 26.08.2020.
//

#include "JpgImageLoader.h"
#include <AUI/Common/AByteBuffer.h>
#include <stb_image.h>

bool JpgImageLoader::matches(AByteBufferView buffer) {
    const uint8_t header[] = {0xff, 0xd8 };
    return memcmp(header, buffer.data(), sizeof(header)) == 0;
}
