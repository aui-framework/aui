
// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "AUI/Common/AException.h"
#include "AUI/Render/ARender.h"
#include "AImageDrawable.h"
#include "AUI/Render/ITexture.h"
#include <AUI/Platform/AWindow.h>
#include <variant>

AImageDrawable::AImageDrawable(_<AImage> image): mSize(image->size()), mStorage(std::move(image)) {
}

AImageDrawable::~AImageDrawable() {

}

glm::ivec2 AImageDrawable::getSizeHint() {
    return mSize;
}


void AImageDrawable::draw(const IDrawable::Params& params) {
    if (auto asImage = std::get_if<_<AImage>>(&mStorage)) {
        auto texture = ARender::getNewTexture();
        texture->setImage(**asImage);
        mStorage = std::move(texture);
    }
    const auto& texture = std::get<_<ITexture>>(mStorage);

    ARender::rect(ATexturedBrush{
            texture,
            params.cropUvTopLeft,
            params.cropUvBottomRight,
            params.imageRendering,
    }, params.offset, params.size);
}

AImage AImageDrawable::rasterize(glm::ivec2 imageSize) {
    if (auto asImage = std::get_if<_<AImage>>(&mStorage)) {
        return (*asImage)->resizedLinearDownscale(imageSize);
    }
    throw AException("unimplemented");
}
