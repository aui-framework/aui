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

SvgDrawable::SvgDrawable(AByteBuffer& data) {
    data.setCurrentPos(data.getSize());
    data << '\0';
    mImage = nsvgParse(data.data(), "px", 96.f);

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
    auto doDraw = [&]() {
        Render::inst().setFill(Render::FILL_TEXTURED);
        glm::vec2 uv = {1, 1};

        float posX, posY;
        float scale = glm::min(size.x / mImage->width, size.y / mImage->height);

        if (Render::inst().getRepeat() & REPEAT_X) {
            uv.x = float(size.x) / getSizeHint().x;
            posX = 0;
        } else {
            posX = glm::round((size.x - mImage->width * scale) / 2.f);
        }
        if (Render::inst().getRepeat() & REPEAT_Y) {
            uv.y = float(size.y) / getSizeHint().y;
            posY = 0;
        } else {
            posY = glm::round((size.y - mImage->height * scale) / 2.f);
        }
        Render::inst().applyTextureRepeat();

        Render::inst().drawTexturedRect(posX,
                                        posY,
                                        size.x,
                                        size.y, {0, 0}, uv);
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

    glm::ivec2 textureSize = size;

    if (Render::inst().getRepeat() & REPEAT_X) {
        textureSize.x = getSizeHint().x;
    }
    if (Render::inst().getRepeat() & REPEAT_Y) {
        textureSize.y = getSizeHint().y;
    }

    // растеризация
    auto texture = _new<GL::Texture2D>();
    auto image = _new<AImage>(AVector<uint8_t>{}, textureSize.x, textureSize.y, AImage::RGBA | AImage::BYTE);
    image->allocate();
    auto rasterizer = nsvgCreateRasterizer();
    assert(rasterizer);
    nsvgRasterize(rasterizer, mImage, 0, 0, glm::min(textureSize.x / mImage->width, textureSize.y / mImage->height),
                  image->getData().data(), textureSize.x, textureSize.y, textureSize.x * 4);

    texture->tex2D(image);
    nsvgDeleteRasterizer(rasterizer);
    mRasterized.push_back({key, texture});
    doDraw();
}
