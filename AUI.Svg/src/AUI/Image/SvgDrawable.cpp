#define NANOSVGRAST_IMPLEMENTATION
#define NANOSVG_IMPLEMENTATION

#include <AUI/Common/AByteBuffer.h>
#include "SvgDrawable.h"

#include <AUI/Common/AString.h>
#include <AUI/Render/Render.h>


inline uint64_t asKey(const glm::ivec2 size) {
    uint64_t result = uint32_t(size.x);
    result <<= 32u;
    result |= uint32_t(size.y);
    return result;
}

SvgDrawable::SvgDrawable(_<AByteBuffer>& data) {
    data->setCurrentPos(data->getSize());
    data << '\0';
    mImage = nsvgParse(data->data(), "px", 96.f);

}

bool SvgDrawable::isDpiDependent() const
{
    return true;
}

SvgDrawable::~SvgDrawable() {
    nsvgDelete(mImage);
}

glm::ivec2 SvgDrawable::getSizeHint() {
    return {mImage->width, mImage->height};
}

void SvgDrawable::draw(const glm::ivec2& size) {
    auto key = asKey(size);
    auto doDraw = [&size]() {
        Render::instance().setFill(Render::FILL_TEXTURED);
        Render::instance().drawTexturedRect(0, 0, size.x, size.y);
    };
    for (auto& p : mRasterized) {
        if (p.key == key) {
            p.texture->bind();
            doDraw();
            return;
        }
    }
    if (mRasterized.size() >= 10) {
        mRasterized.pop_front();
    }

    // растеризация
    auto texture = _new<GL::Texture>();
    auto image = _new<AImage>(AVector<uint8_t>{}, size.x, size.y, AImage::RGBA | AImage::BYTE);
    image->allocate();
    auto rasterizer = nsvgCreateRasterizer();
    assert(rasterizer);
    nsvgRasterize(rasterizer, mImage, 0, 0, glm::max(size.x / mImage->width, size.y / mImage->height),
                  image->getData().data(), size.x, size.y, size.x * 4);

    texture->tex2D(image);
    nsvgDeleteRasterizer(rasterizer);
    mRasterized.push_back({key, texture});
    doDraw();
}
