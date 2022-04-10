/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
    texture->setImage(mFactory->provideImage(textureSize));
    mRasterized.push_back({key, texture});
    doDraw(texture);
}
