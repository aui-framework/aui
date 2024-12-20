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
// Created by alex2772 on 25.07.2018.
//

#include <cassert>
#include <cstring>
#include "Texture2DArray.h"
#include "gl.h"
#include "TextureImpl.h"

template class gl::Texture<gl::TEXTURE_2D_ARRAY>;

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
}

inline bool Result::operator==(const Result& rhs) const
{
	return memcmp(this, &rhs, sizeof(rhs)) == 0;
}

static Result recognize(AImageView image)
{
	Result res;
	switch (image.format() & APixelFormat::COMPONENT_BITS)
	{
	case APixelFormat::R:
		res.format = GL_RED;
		switch (image.format() & APixelFormat::TYPE_BITS)
		{
		case APixelFormat::FLOAT:
			res.internalformat = GL_R16F;
			res.type = GL_FLOAT;
			break;
		case APixelFormat::BYTE:
			res.internalformat = GL_R8;
			res.type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(0);
		}
		break;
	case APixelFormat::RGB:
		res.format = GL_RGB;
		switch (image.format() & APixelFormat::TYPE_BITS)
		{
		case APixelFormat::FLOAT:
			res.internalformat = GL_RGB16F;
			res.type = GL_FLOAT;
			break;
		case APixelFormat::BYTE:
			res.internalformat = GL_RGB;
			res.type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(0);
		}
		break;
	case APixelFormat::RGBA:
		res.format = GL_RGBA;
		switch (image.format() & APixelFormat::TYPE_BITS)
		{
		case APixelFormat::FLOAT:
			res.internalformat = GL_RGBA16F;
			res.type = GL_FLOAT;
			break;
		case APixelFormat::BYTE:
			res.internalformat = GL_RGBA;
			res.type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(0);
		}
		break;
	default:
		assert(0);
	}
	return res;
}

void gl::Texture2DArray::tex3D(const AVector<AImageView>& images) {
    static constexpr auto MIPMAP_LEVELS = 1;
    bind();
    if (images.empty()) {
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, MIPMAP_LEVELS, GL_RGBA8, 0, 0, 0);
        return;
    }

    Result types = recognize(images.first());

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, MIPMAP_LEVELS, GL_RGBA8, images.first().width(), images.first().height(), images.size());

    for (size_t i = 0; i < images.size(); ++i) {
        auto image = images[i];
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(i), image.width(), image.height(), 1, types.format, types.type, image.data());
    }
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}
