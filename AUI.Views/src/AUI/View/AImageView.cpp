//
// Created by alex2 on 27.08.2020.
//

#include "AImageView.h"
#include <AUI/Render/Render.h>
#include <AUI/View/AViewContainer.h>

AImageView::AImageView(const _<GL::Texture>& texture) : mTexture(texture) {}

AImageView::AImageView(const _<AImage>& img) {
    mTexture = _new<GL::Texture>();
    mTexture->tex2D(img);
    mImageSize = img->getSize();
}

AImageView::AImageView() {

}


void AImageView::render() {
    AView::render();
    if (mTexture) {
        mTexture->bind();
        Render::inst().setFill(Render::FILL_TEXTURED);
        Render::inst().drawTexturedRect(0, 0, getContentWidth(), getContentHeight());
    }
}

int AImageView::getContentMinimumWidth() {
    return 0;
}

int AImageView::getContentMinimumHeight() {
    return getContentWidth() * mImageSize.y / mImageSize.x;
}

void AImageView::setSize(int width, int height) {
    auto w = getSize().x != width;
    AView::setSize(width, height);
    if (w && mFixedSize.x == 0)
        getParent()->updateLayout();
}

bool AImageView::consumesClick(const glm::ivec2& pos) {
    return false;
}
