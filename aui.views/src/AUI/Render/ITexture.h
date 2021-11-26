#pragma once

#include <AUI/Image/AImage.h>


class ITexture {
public:
    virtual void setImage(const _<AImage>& image) = 0;
    virtual ~ITexture() = default;
};
