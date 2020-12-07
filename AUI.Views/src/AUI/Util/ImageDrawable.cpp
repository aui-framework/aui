//
// Created by alex2 on 23.10.2020.
//

#include <AUI/Render/Render.h>
#include "ImageDrawable.h"
#include <AUI/Platform/AWindow.h>

ImageDrawable::ImageDrawable(const _<AImage> image): mSize(image->getSize()) {
    mTexture.tex2D(image);
}

ImageDrawable::~ImageDrawable() {

}

void ImageDrawable::draw(const glm::ivec2& size) {
    mTexture.bind();
    Render::inst().setFill(Render::FILL_TEXTURED);
    Render::inst().drawTexturedRect(0, 0, size.x, size.y);
}

glm::ivec2 ImageDrawable::getSizeHint() {
    return mSize;
}
