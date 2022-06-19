/*
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
	switch (image.getFormat() & 15)
	{
	case AImage::R:
		res.format = GL_RED;
		switch (image.getFormat() & (~15))
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
		switch (image.getFormat() & (~15))
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
		switch (image.getFormat() & (~15))
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
