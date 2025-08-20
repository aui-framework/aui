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

template class gl::Texture<gl::TEXTURE_2D_ARRAY>;

static constexpr auto MIPMAP_LEVELS = 1;

namespace {
/**
 * @internal
 */
struct Result {
    /*
     * GL_R16F / GL_RGB16F / GL_RGBA16F / GL_RGBA ....
     */
    GLint internalformat = 0;

    /*
     * GL_RED / GL_RGB / GL_RGBA
     */
    GLenum format = 0;

    /*
     * GL_FLOAT / GL_UNSIGNED_BYTE
     */
    GLenum type = GL_FLOAT;

    bool operator==(const Result& rhs) const;
};
}   // namespace

inline bool Result::operator==(const Result& rhs) const { return memcmp(this, &rhs, sizeof(rhs)) == 0; }

static Result recognize(AImageView image) {
    Result r{};
    const auto comps = (image.format() & APixelFormat::COMPONENT_BITS);
    const auto type = (image.format() & APixelFormat::TYPE_BITS);
    switch (comps) {
        case APixelFormat::R:
            r.format = GL_RED;
            break;
        case APixelFormat::RG:
            r.format = GL_RG;
            break;
        case APixelFormat::RGB:
            r.format = GL_RGB;
            break;
        case APixelFormat::RGBA:
            r.format = GL_RGBA;
            break;
        case APixelFormat::BGRA:
            #if defined(GL_BGRA_EXT) && !defined(GL_BGRA)
            r.format = GL_BGRA_EXT;
            #elif defined(GL_BGRA)
            r.format = GL_BGRA;
            #else
            ALogger::warn("Texture2D") << "Unhandled components mask for BGRA: 0x" << std::hex << comps << " (defaulting to RGBA)";
            r.format = GL_RGBA;
            #endif
            break;
        default:
            ALogger::warn("Texture2D") << "Unhandled components mask: 0x" << std::hex << comps << " (defaulting to RGBA)";
            r.format = GL_RGBA;
            break;
    }
    switch (type) {
        case APixelFormat::BYTE:
            r.type = GL_UNSIGNED_BYTE;
            if (comps == APixelFormat::R)
                r.internalformat = GL_R8;
            else if (comps == APixelFormat::RG)
                r.internalformat = GL_RG8;
            else
                r.internalformat = GL_RGBA8;
            break;
        case APixelFormat::FLOAT:
            r.type = GL_FLOAT;
            if (comps == APixelFormat::R)
                r.internalformat = GL_R32F;
            else if (comps == APixelFormat::RG)
                r.internalformat = GL_RG32F;
            else if (comps == APixelFormat::RGB)
                r.internalformat = GL_RGB32F;
            else
                r.internalformat = GL_RGBA32F;
            break;
        default:
            ALogger::warn("Texture2D") << "Unhandled type mask: 0x" << std::hex << type << " (defaulting to UBYTE RGBA8)";
            r.type = GL_UNSIGNED_BYTE;
            r.internalformat = (comps == APixelFormat::R) ? GL_R8 : (comps == APixelFormat::RGB) ? GL_RGB8 : GL_RGBA8;
            break;
    }
    return r;
}

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

    Result types = recognize(image);

    glTexSubImage3D(
        GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(texture), image.width(), image.height(), 1, types.format, types.type,
        image.data());
}
