//
// Created by dervisdev on 1/11/2023.
//

#include "GifImageLoader.h"
#include "GifImageFactory.h"

bool GifImageLoader::matches(AByteBufferView buffer) {
    return memcmp(buffer.data(), "GIF87a", 6) == 0 ||
            memcmp(buffer.data(), "GIF89a", 6) == 0;
}

_<IImageFactory> GifImageLoader::getImageFactory(AByteBufferView buffer) {
    return _new<GifImageFactory>(buffer);
}

_<AImage> GifImageLoader::getRasterImage(AByteBufferView buffer) {
    return nullptr;
}