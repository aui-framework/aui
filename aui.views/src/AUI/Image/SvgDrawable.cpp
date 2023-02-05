﻿// AUI Framework - Declarative UI toolkit for modern C++20
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

#define NANOSVGRAST_IMPLEMENTATION
#define NANOSVG_IMPLEMENTATION

#include <AUI/Common/AByteBuffer.h>
#include "AVectorDrawable.h"
#include "AUI/Image/IImageFactory.h"

#include <AUI/Common/AString.h>
#include <AUI/Render/Render.h>


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

void AVectorDrawable::draw(const Params& params) {
    auto& size = params.size;
    auto key = asKey(size);
    auto doDraw = [&](const Render::Texture& texture) {
        bool uvModifiedFlag = false;
        glm::vec2 uv = {1, 1};

        glm::vec2 pos;
        auto sizeHint = glm::vec2(getSizeHint());
        float scale = glm::min(size.x / sizeHint.x, size.y / sizeHint.y);

        if (!!(params.repeat & Repeat::X)) {
            uv.x = float(size.x) / getSizeHint().x;
            uvModifiedFlag = true;
            pos.x = 0;
        } else {
            pos.x = glm::round((size.x - sizeHint.x * scale) / 2.f);
        }
        if (!!(params.repeat & Repeat::Y)) {
            uv.y = float(size.y) / getSizeHint().y;
            uvModifiedFlag = true;
            pos.y = 0;
        } else {
            pos.y = glm::round((size.y - sizeHint.y * scale) / 2.f);
        }
        pos += params.offset;
        Render::rect(ATexturedBrush{
                             texture,
                             std::nullopt,
                             uvModifiedFlag ? static_cast<decltype(ATexturedBrush::uv1)>(uv) : std::nullopt,
                             params.imageRendering,
                             params.repeat,
                     },
                     pos,
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

    glm::ivec2 textureSize = size;

    if (!!(params.repeat & Repeat::X)) {
        textureSize.x = getSizeHint().x;
    }
    if (!!(params.repeat & Repeat::Y)) {
        textureSize.y = getSizeHint().y;
    }

    // rasterization
    auto texture = Render::getNewTexture();
    texture->setImage(_new<AImage>(mFactory->provideImage(textureSize)));
    mRasterized.push_back({key, texture});
    doDraw(texture);
}

AImage AVectorDrawable::rasterize(glm::ivec2 imageSize) {
    return mFactory->provideImage(imageSize);
}
