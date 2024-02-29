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
