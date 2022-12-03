// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2 on 26.08.2020.
//

#include "PngImageLoader.h"
#include <AUI/Common/AByteBuffer.h>

#include <stb_image.h>
#include <stb_image_write.h>

bool PngImageLoader::matches(AByteBufferView buffer) {
    const uint8_t png_header[] = {0x89, 0x50, 0x4e, 0x47};
    return memcmp(png_header, buffer.data(), sizeof(png_header)) == 0;
}


void PngImageLoader::save(IOutputStream& outputStream, const AImage& image) {
    stbi_write_png_to_func([](void *context, void *data, int size) {
        reinterpret_cast<IOutputStream*>(context)->write(reinterpret_cast<char*>(data), size);
    }, reinterpret_cast<void*>(&outputStream), image.getWidth(), image.getHeight(), image.getBytesPerPixel(), image.getData().data(), image.getWidth() * image.getBytesPerPixel());
}
