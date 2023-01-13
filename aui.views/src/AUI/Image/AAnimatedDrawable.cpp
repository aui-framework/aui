//
// Created by dervisdev on 1/12/2023.
//

#include "AAnimatedDrawable.h"

void AAnimatedDrawable::draw(const IDrawable::Params &params) {
    auto texture = Render::getNewTexture();
    auto img = _new<AImage> (mFactory->provideImage(params.size));
    texture->setImage(img);
    Render::rect(ATexturedBrush{
            texture,
            params.cropUvTopLeft,
            params.cropUvBottomRight,
            params.imageRendering,
    }, params.offset, params.size);
}

glm::ivec2 AAnimatedDrawable::getSizeHint() {
    return glm::ivec2();
}
