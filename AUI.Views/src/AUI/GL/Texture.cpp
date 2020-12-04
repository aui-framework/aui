//
// Created by Алексей on 25.07.2018.
//

#include <cassert>
#include <cstring>
#include "Texture.h"
#include "gl.h"
#include "State.h"
#include "gl.h"

struct Result
{
	/**
	 * GL_R16F / GL_RGB16F / GL_RGBA16F / GL_RGBA ....
	 */
	GLint internalformat = 0;

	/**
	 * GL_RED / GL_RGB / GL_RGBA
	 */
	GLenum format = 0;

	/**
	 * GL_FLOAT / GL_UNSIGNED_BYTE
	 */
	GLenum type = GL_FLOAT;

	bool operator==(const Result& rhs) const;
};

bool Result::operator==(const Result& rhs) const
{
	return memcmp(this, &rhs, sizeof(rhs)) == 0;
}

Result recognize(const _<AImage>& image)
{
	Result res;
	switch (image->getFormat() & 15)
	{
	case AImage::R:
		res.format = GL_RED;
		switch (image->getFormat() & (~15))
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
		switch (image->getFormat() & (~15))
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
		switch (image->getFormat() & (~15))
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

GL::Texture::Texture() {
	glGenTextures(1, &mTexture);
	bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

GL::Texture::~Texture() {

	glDeleteTextures(1, &mTexture);
	GL::State::bindTexture(GL_TEXTURE_2D, 0);
}

void GL::Texture::setupNearest()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GL::Texture::bind(uint8_t index) const {
	GL::State::activeTexture(index);
	GL::State::bindTexture(GL_TEXTURE_2D, mTexture);
}

void GL::Texture::tex2D(_<AImage> image) {
	bind();
	Result types = recognize(image);

	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, types.internalformat, image->getWidth(), image->getHeight(), 0, types.format, types.type, image->getData().empty() ? nullptr : image->getData().data());
	assert(glGetError() == 0);
}

GL::Texture::operator bool() const {
	return mTexture > 0;
}

uint32_t GL::Texture::getHandle() {
	return mTexture;
}

