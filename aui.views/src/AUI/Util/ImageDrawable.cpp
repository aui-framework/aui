
/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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


void AImageDrawable::draw(IRenderer& render, const IDrawable::Params& params) {
    if (auto asImage = std::get_if<_<AImage>>(&mStorage)) {
        auto texture = render.getNewTexture();
        texture->setImage(**asImage);
        mStorage = std::move(texture);
    }
    const auto& texture = std::get<_<ITexture>>(mStorage);

    render.rectangle(ATexturedBrush{
            .texture = texture,
            .uv1 = params.cropUvTopLeft,
            .uv2 = params.cropUvBottomRight,
            .imageRendering = params.imageRendering,
            .repeat = params.repeat,
    }, params.offset, params.size);
}

AImage AImageDrawable::rasterize(glm::ivec2 imageSize) {
    if (auto asImage = std::get_if<_<AImage>>(&mStorage)) {
        return (*asImage)->resizedLinearDownscale(imageSize);
    }
    throw AException("unimplemented");
}
