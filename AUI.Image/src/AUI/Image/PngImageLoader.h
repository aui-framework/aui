//
// Created by alex2 on 26.08.2020.
//

#pragma once


#include <AUI/Image/IImageLoader.h>

class PngImageLoader: public IImageLoader {
public:
    bool matches(_<ByteBuffer> buffer) override;

    _<IDrawable> getDrawable(_<ByteBuffer> buffer) override;
    _<AImage> getRasterImage(_<ByteBuffer> buffer) override;
};


