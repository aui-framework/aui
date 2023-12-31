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

#pragma once

#include "AUI/Enum/ImageRendering.h"
#include "AUI/Image/AImage.h"
#include "glm/common.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <AUI/Render/ITexture.h>

namespace aui::sl_gen {
    struct Texture2D {
    public:
        Texture2D(AImageView texture, ImageRendering imageRendering) noexcept: mTexture(texture), mImageRendering(imageRendering) {}
        Texture2D(AImageView texture) noexcept: Texture2D(texture, ImageRendering::PIXELATED) {}

        glm::vec4 operator[](glm::vec2 uv) const {
            switch (mImageRendering) {
                case ImageRendering::PIXELATED:
                    return clamped(glm::ivec2(uv * glm::vec2(mTexture.size())));
                case ImageRendering::SMOOTH: {
                    const auto pos = glm::ivec2(uv * glm::vec2(mTexture.size()));
                    const auto texelSize = glm::vec2(1.f) / glm::vec2(mTexture.size());
                    const auto topLeft = clamped(pos);
                    const auto topRight = clamped(pos + glm::ivec2(1, 0));
                    const auto bottomLeft = clamped(pos + glm::ivec2(0, 1));
                    const auto bottomRight = clamped(pos + glm::ivec2(1, 1));
                    const auto texelSpaceUv = glm::mod(uv, texelSize) / texelSize;

                    const auto top = glm::mix(topLeft, topRight, texelSpaceUv.x);
                    const auto bottom = glm::mix(bottomLeft, bottomRight, texelSpaceUv.x);
                    return glm::mix(top, bottom, texelSpaceUv.y);
                }
            }
        }

    private:
        glm::vec4 clamped(glm::ivec2 pos) const noexcept {
            return mTexture.get(glm::clamp(pos, glm::ivec2(0), glm::ivec2(mTexture.size()) - 1));
        }
        AImageView mTexture;
        ImageRendering mImageRendering;
    };
}

    inline glm::vec2 gl_SamplePosition = {0, 0};

