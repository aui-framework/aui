/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 26.08.2020.
//

#include "JpgImageLoader.h"
#include <AUI/Common/AByteBuffer.h>
#include <stb_image.h>

bool JpgImageLoader::matches(AByteBuffer& buffer) {
    const uint8_t header[] = {0xff, 0xd8 };
    uint8_t read_header[sizeof(header)];
    buffer.get((char*) read_header, sizeof(read_header));
    return memcmp(header, read_header, sizeof(read_header)) == 0;
}

_<IDrawable> JpgImageLoader::getDrawable(AByteBuffer& buffer) {
    return nullptr;
}

_<AImage> JpgImageLoader::getRasterImage(AByteBuffer& buffer) {
    int x, y, channels;
    if (stbi_uc* data = stbi_load_from_memory((const stbi_uc*) buffer.getCurrentPosAddress(), buffer.getAvailable(),
                                              &x, &y, &channels, 4)) {
        channels = 4;
        uint32_t format = AImage::BYTE;
        switch (channels) {
            case 3:
                format |= AImage::RGB;
                break;
            case 4:
                format |= AImage::RGBA;
                break;
            default:
                assert(0);
        }
        auto img = _new<AImage>(
                AVector<uint8_t>{static_cast<uint8_t*>(data), static_cast<uint8_t*>(data + x * y * channels)}, x, y,
                format);
        stbi_image_free(data);
        return img;
    }
    return nullptr;
}
