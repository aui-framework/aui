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

#pragma once

#include "AUI/Common/SharedPtr.h"
#include "AUI/Image/AImage.h"
#include "Texture.h"
#include "GLEnums.h"

namespace gl {
class API_AUI_VIEWS Texture2D final: public Texture<TEXTURE_2D> {
public:
    void tex2D(AImageView image);
    virtual ~Texture2D() = default;
    void framebufferTex2D(glm::u32vec2 size, gl::Type type);

private:
    glm::u32vec2 mSize = { 0, 0 };
};
}   // namespace gl
