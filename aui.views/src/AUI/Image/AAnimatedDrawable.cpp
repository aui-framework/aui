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
