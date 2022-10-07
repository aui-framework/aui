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
        auto img = _new<AImage>(AByteBuffer(reinterpret_cast<const char*>(data), x * y * channels), x, y, format);

        return img;
    }
    return nullptr;
}
