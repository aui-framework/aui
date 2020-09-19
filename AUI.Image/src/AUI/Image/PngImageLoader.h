//
// Created by alex2 on 26.08.2020.
//

#pragma once


#include <AUI/Image/IImageLoader.h>

class PngImageLoader: public IImageLoader {
public:
    bool matches(_<AByteBuffer> buffer) override;

    _<IDrawable> getDrawable(_<AByteBuffer> buffer) override;
    _<AImage> getRasterImage(_<AByteBuffer> buffer) override;
};


