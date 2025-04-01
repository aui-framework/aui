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
// Created by Alex2772 on 10/4/2022.
//

#include "StbImageLoader.h"

#include <stb_image.h>
#include <stb_image_write.h>
#include <AUI/Util/ARaiiHelper.h>

_<AImage> StbImageLoader::getRasterImage(AByteBufferView buffer) {
    int x, y, channels;
    if (stbi_uc* data = stbi_load_from_memory((const stbi_uc*) buffer.data(), buffer.size(),
                                              &x, &y, &channels, 4)) {
        ARaiiHelper dealloc = [&] { stbi_image_free(data); };
        channels = 4;
        unsigned format = APixelFormat::BYTE;
        switch (channels) {
            case 3:
                format |= APixelFormat::RGB;
                break;
            case 4:
                format |= APixelFormat::RGBA;
                break;
            default:
                AUI_ASSERT(0);
        }
        auto img = _new<AImage>(AByteBufferView(reinterpret_cast<const char*>(data), x * y * channels), glm::uvec2{x, y}, format);

        return img;
    }
    return nullptr;
}
