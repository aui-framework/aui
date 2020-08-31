//
// Created by alex2 on 27.08.2020.
//

#include "AImageView.h"
#include <AUI/Render/Render.h>

AImageView::AImageView(const _<GL::Texture>& texture) : mTexture(texture) {}

AImageView::AImageView(const _<AImage>& img) {
    mTexture = _new<GL::Texture>();
    mTexture->tex2D(img);
}

void AImageView::render() {
    AView::render();
    mTexture->bind();
    Render::instance().setFill(Render::FILL_TEXTURED);
    Render::instance().drawTexturedRect(0, 0, getWidth(), getHeight());
}

void AImageView::setSize(int width, int height) {
    auto size = glm::max(width, height);
    AView::setSize(size, size);
}

int AImageView::getContentMinimumWidth() {
    return 10;
}

int AImageView::getContentMinimumHeight() {
    return 10;
}
