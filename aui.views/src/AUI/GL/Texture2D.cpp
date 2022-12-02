// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

Result recognize(const AImage& image)
{
	Result res;
	switch (image.getFormat() & AImage::STRUCTURE)
	{
	case AImage::R:
		res.format = GL_RED;
		switch (image.getFormat() & (~AImage::STRUCTURE))
		{
		case AImage::FLOAT:
			res.internalformat = GL_R16F;
			res.type = GL_FLOAT;
			break;
		case AImage::BYTE:
			res.internalformat = GL_R8;
			res.type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(0);
		}
		break;
	case AImage::RGB:
		res.format = GL_RGB;
		switch (image.getFormat() & (~AImage::STRUCTURE))
		{
		case AImage::FLOAT:
			res.internalformat = GL_RGB16F;
			res.type = GL_FLOAT;
			break;
		case AImage::BYTE:
			res.internalformat = GL_RGB;
			res.type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(0);
		}
		break;
	case AImage::RGBA:
		res.format = GL_RGBA;
		switch (image.getFormat() & (~AImage::STRUCTURE))
		{
		case AImage::FLOAT:
			res.internalformat = GL_RGBA16F;
			res.type = GL_FLOAT;
			break;
		case AImage::BYTE:
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

void gl::Texture2D::tex2D(const AImage& image) {
	bind();
	Result types = recognize(image);

	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, types.internalformat, image.getWidth(), image.getHeight(), 0, types.format, types.type, image.getData().empty() ? nullptr : image.getData().data());
	assert(glGetError() == 0);
}
