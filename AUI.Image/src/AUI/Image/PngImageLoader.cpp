//
// Created by alex2 on 26.08.2020.
//

#include "PngImageLoader.h"
#include <AUI/Common/AByteBuffer.h>

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool PngImageLoader::matches(_<AByteBuffer> buffer) {
    const uint8_t png_header[] = {0x89, 0x50, 0x4e, 0x47};
    uint8_t read_header[sizeof(png_header)];
    buffer->get((char*) read_header, sizeof(read_header));
    return memcmp(png_header, read_header, sizeof(read_header)) == 0;
}

_<IDrawable> PngImageLoader::getDrawable(_<AByteBuffer> buffer) {
    return nullptr;
}

_<AImage> PngImageLoader::getRasterImage(_<AByteBuffer> buffer) {
    int x, y, channels;
    if (stbi_uc* data = stbi_load_from_memory((const stbi_uc*) buffer->getCurrentPosAddress(), buffer->getAvailable(),
                                              &x, &y, &channels, 0)) {
        assert(channels == 3 || channels == 4);
        auto img = _new<AImage>(
                AVector<uint8_t>{static_cast<uint8_t*>(data), static_cast<uint8_t*>(data + x * y * channels)}, x, y,
                channels | AImage::BYTE);
        stbi_image_free(data);
        return img;
    }
    return nullptr;
}
