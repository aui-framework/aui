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
// Created by alex2772 on 25.07.2018.
//

#include <cassert>
#include <cstring>
#include "Texture2DArray.h"
#include "gl.h"
#include "TextureImpl.h"
#include "Texture2D.h"
#include "TextureFormatRecognition.h"

template class gl::Texture<gl::TEXTURE_2D_ARRAY>;

static constexpr auto MIPMAP_LEVELS = 1;

gl::Texture2DArray::Texture2DArray(glm::uvec2 textureSize, unsigned int textureCount) {
    bind();
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, MIPMAP_LEVELS, GL_RGBA8, textureSize.x, textureSize.y, textureCount);
    mTextureSize = textureSize;
    mTextureCount = textureCount;
}

void gl::Texture2DArray::update(unsigned int texture, AImageView image) {
    bind();
    AUI_ASSERT(texture < mTextureCount);
    AUI_ASSERT(image.size() == mTextureSize);

    auto types = aui::gl::impl::recognize(image);

    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(texture), image.width(), image.height(), 1, types.format, types.type,
        image.data());
}
