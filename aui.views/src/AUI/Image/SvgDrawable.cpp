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

#define NANOSVGRAST_IMPLEMENTATION
#define NANOSVG_IMPLEMENTATION

#include <AUI/Common/AByteBuffer.h>
#include "AVectorDrawable.h"
#include "AUI/Image/IImageFactory.h"

#include <AUI/Common/AString.h>
#include <AUI/Render/IRenderer.h>
#include <AUI/Render/ACanvas.hpp>
#include <AUI/Render/IRendererBackend.h>


inline uint64_t asKey(const glm::ivec2 size) {
    uint64_t result = uint32_t(size.x);
    result <<= 32u;
    result |= uint32_t(size.y);
    return result;
}

bool AVectorDrawable::isDpiDependent() const
{
    return true;
}

AVectorDrawable::~AVectorDrawable() = default;

glm::ivec2 AVectorDrawable::getSizeHint() {
    return mFactory->getSizeHint();
}

void AVectorDrawable::draw(ARenderContext ctx, const IDrawable::Params& params) {
    auto& size = params.size;
    if (size.x < 1 || size.y < 1) {
        return;
    }
    auto key = asKey(size);
    auto doDraw = [&](const _<ITexture>& texture) {
        ctx.canvas.rectangle(APaint{ATexturedBrush{
                                     .texture = texture,
                                     .uv1 = params.cropUvTopLeft,
                                     .uv2 = params.cropUvBottomRight,
                                     .imageRendering = ImageRendering::PIXELATED,
                                     .repeat = params.repeat,
                             }},
                             params.offset,
                             size);
    };
    for (auto& p : mRasterized) {
        if (p.key == key) {
            doDraw(p.texture);
            return;
        }
    }
    if (mRasterized.size() >= 10) {
        mRasterized.pop_front();
    }

    glm::ivec2 textureSize = params.renderingSize.valueOr(size);

    if (!!(params.repeat & Repeat::X)) {
        textureSize.x = getSizeHint().x;
    }
    if (!!(params.repeat & Repeat::Y)) {
        textureSize.y = getSizeHint().y;
    }

    auto& backend = ctx.backend;
    auto img = mFactory->provideImage(glm::max(textureSize, glm::ivec2(0)));
    auto texture = backend.createTexture(img.size(), img.format(), TextureFilter::NEAREST);
    texture->upload(img);
    mRasterized.push_back({key, texture});
    doDraw(texture);
}

AImage AVectorDrawable::rasterize(glm::ivec2 imageSize) {
    return mFactory->provideImage(imageSize);
}
