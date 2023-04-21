
// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 23.10.2020.
//

#include <AUI/Render/Render.h>
#include "AImageDrawable.h"
#include <AUI/Platform/AWindow.h>

AImageDrawable::AImageDrawable(_<AImage> image): mSize(image->size()), mImage(std::move(image)) {
    mTexture = Render::getNewTexture();
    mTexture->setImage(mImage);
}

AImageDrawable::~AImageDrawable() {

}

glm::ivec2 AImageDrawable::getSizeHint() {
    return mSize;
}


void AImageDrawable::draw(const IDrawable::Params& params) {
    Render::rect(ATexturedBrush{
            mTexture,
            params.cropUvTopLeft,
            params.cropUvBottomRight,
            params.imageRendering,
    }, params.offset, params.size);
}

AImage AImageDrawable::rasterize(glm::ivec2 imageSize) {
    return mImage->resizedLinearDownscale(imageSize);
}
