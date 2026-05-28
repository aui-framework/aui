/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by dervisdev on 1/12/2023.
//

#include "AAnimatedDrawable.h"
#include "AUI/Image/gif/GifImageFactory.h"
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/IRendererBackend.h>

AAnimatedDrawable::AAnimatedDrawable(_<IAnimatedImageFactory> factory) : mFactory (std::move(factory)) {
}

void AAnimatedDrawable::draw(ACanvas& render, const IDrawable::Params& params) {
    APerformanceSection s("AAnimatedDrawable::draw");

    if (mFactory->isNewImageAvailable()) {
        auto img = [&] {
            APerformanceSection s2("provideImage");
            return mFactory->provideImage(params.size);
        }();

        if (!mTexture || mTexture->getSize() != glm::u32vec2(img.size())) {
            auto& backend = render.renderer();
            mTexture = backend.createTexture(img.size(), img.format(), TextureFilter::NEAREST);
        }

        if (mFactory->hasAnimationFinished()) {
            emit animationFinished;
        }
        APerformanceSection s2("upload");
        mTexture->upload(img);
    }

    APerformanceSection s2("draw");
    render.rectangle(APaint{ATexturedBrush{
            mTexture,
            params.cropUvTopLeft,
            params.cropUvBottomRight,
            params.imageRendering,
    }}, params.offset, params.size);
}

glm::ivec2 AAnimatedDrawable::getSizeHint() {
    return mFactory->getSizeHint();
}
