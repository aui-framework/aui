#pragma once

#include <AUI/api.h>
#include <AUI/Image/StbImageLoader.h>

class GifImageLoader : public IImageLoader {
public:
    GifImageLoader() = default;
    virtual ~GifImageLoader() = default;

    bool matches(AByteBufferView buffer) override;
    _<AImage> getRasterImage(AByteBufferView buffer) override;
    _<IImageFactory> getImageFactory(AByteBufferView buffer) override;
};

