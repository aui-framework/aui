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

//
// Created by alex2772 on 25.07.2018.
//

#include <cassert>
#include <cstring>
#include "Texture2D.h"
#include "gl.h"
#include "TextureImpl.h"

template class gl::Texture<gl::TEXTURE_2D>;

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
void gl::Texture2D::framebufferTex2D(glm::u32vec2 size, gl::Type type) {
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, static_cast<GLenum>(type), nullptr);
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

void gl::Texture2D::tex2D(AImageView image) {
	bind();
	Result types = recognize(image);

	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, types.internalformat, image.width(), image.height(), 0, types.format, types.type, image.buffer().empty() ? nullptr : image.buffer().data());
	assert(glGetError() == 0);
}
