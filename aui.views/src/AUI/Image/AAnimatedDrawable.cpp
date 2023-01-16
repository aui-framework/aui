//
// Created by dervisdev on 1/12/2023.
//

#include "AAnimatedDrawable.h"

void AAnimatedDrawable::draw(const IDrawable::Params &params) {
    if (!mTexture)
        mTexture = Render::getNewTexture();

    if (mFactory->isNewImageAvailable()) {
        auto img = _new<AImage>(mFactory->provideImage(params.size));
        mTexture->setImage(img);
    }

    Render::rect(ATexturedBrush{
            mTexture,
            params.cropUvTopLeft,
            params.cropUvBottomRight,
            params.imageRendering,
    }, params.offset, params.size);
}

glm::ivec2 AAnimatedDrawable::getSizeHint() {
    return glm::ivec2();
}
