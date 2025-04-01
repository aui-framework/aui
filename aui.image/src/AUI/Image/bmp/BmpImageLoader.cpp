/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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

#include "BmpImageLoader.h"
#include <AUI/Common/AByteBuffer.h>
#include <stb_image_write.h>

bool BmpImageLoader::matches(AByteBufferView buffer) {
    const uint8_t bmp_header[] = {'B', 'M'};
    return memcmp(bmp_header, buffer.data(), sizeof(bmp_header)) == 0;
}

void BmpImageLoader::save(aui::no_escape<IOutputStream> outputStream, AImageView image) {
    stbi_write_bmp_to_func([](void *context, void *data, int size) {
      reinterpret_cast<IOutputStream*>(context)->write(reinterpret_cast<char*>(data), size);
    }, reinterpret_cast<void*>(outputStream.ptr()), image.width(), image.height(), image.bytesPerPixel(), image.data());
}
