//
// Created by Alex2772 on 2/8/2022.
//

#include "SvgImageFactory.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg.h>
#include <nanosvgrast.h>


SvgImageFactory::SvgImageFactory(const AByteBuffer& buf) {
    auto copy = buf;
    copy << '\0';
    mNsvg = nsvgParse(copy.data(), "px", 96.f);
}

SvgImageFactory::~SvgImageFactory() {
    nsvgDelete(static_cast<NSVGimage*>(mNsvg));
}

_<AImage> SvgImageFactory::provideImage(const glm::ivec2& size) {
    auto image = _new<AImage>(AVector<uint8_t>{}, size.x, size.y, AImage::RGBA | AImage::BYTE);
    image->allocate();
    auto rasterizer = nsvgCreateRasterizer();
    assert(rasterizer);
    auto svg = static_cast<NSVGimage*>(mNsvg);
    nsvgRasterize(rasterizer, svg, 0, 0, glm::min(size.x / svg->width, size.y / svg->height),
                  image->getData().data(), size.x, size.y, size.x * 4);
    nsvgDeleteRasterizer(rasterizer);
    return image;
}

glm::ivec2 SvgImageFactory::getSizeHint() {
    auto svg = static_cast<NSVGimage*>(mNsvg);
    return {svg->width, svg->height};
}
