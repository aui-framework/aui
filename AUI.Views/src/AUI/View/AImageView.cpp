/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 27.08.2020.
//

#include "AImageView.h"
#include <AUI/Render/Render.h>
#include <AUI/View/AViewContainer.h>

AImageView::AImageView(const _<GL::Texture2D>& texture) : mTexture(texture) {}

AImageView::AImageView(const _<AImage>& img) {
    mTexture = _new<GL::Texture2D>();
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
