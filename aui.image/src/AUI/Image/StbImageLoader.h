#pragma once


#include "IImageLoader.h"

class StbImageLoader: public IImageLoader {
public:
    _<AImage> getRasterImage(AByteBufferView buffer) override;
};


