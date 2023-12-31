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

#include "Vao.h"
#include <cstdint>
#include "State.h"



gl::Vao::Vao() {
    glGenVertexArrays(1, &mHandle);
	bind();
}

gl::Vao::~Vao() {
	auto buffers = mBuffers;
	auto handle = mHandle;

	gl::State::bindVertexArray(handle);
	glDeleteBuffers(1, &mIndicesBuffer);
	for (auto v : buffers) {
		glDeleteBuffers(1, &v.handle);
	}
	glDeleteVertexArrays(1, &handle);
	gl::State::bindVertexArray(0);
}

void gl::Vao::bind() const noexcept {
	gl::State::bindVertexArray(mHandle);
}

void gl::Vao::unbind() noexcept {
	gl::State::bindVertexArray(0);
}

void gl::Vao::drawArrays(GLenum type, GLsizei count) {
	bind();
	glDrawArrays(type, 0, count);
}

void gl::Vao::insertIfKeyMismatches(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType, const char* key) {
	if (mBuffers.size() <= index) {
		goto goAhead;
	}
	if (mBuffers[index].lastModifierKey != key) {
		goto goAhead;
	}
	return;

	goAhead:
	insert(index, data, dataSize, vertexSize, dataType, key);
}
void gl::Vao::insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType, const char* key) {
	bind();
	bool newFlag = true;
	if (mBuffers.size() <= index) {
		mBuffers.resize(index + 1);
		glGenBuffers(1, &mBuffers[index].handle);
		assert(mBuffers[index].handle);
	} else if (mBuffers[index].handle == 0)
	{
		glGenBuffers(1, &mBuffers[index].handle);
		assert(mBuffers[index].handle);
	} else
	{
		newFlag = false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, mBuffers[index].handle);

	// You can use the glBufferSubData function to update buffer contents, but doing so incurs a performance penalty
	// because it flushes the command buffer and waits for all commands to complete.
	//
	// Complex UI tests showed better performance on glBufferData.
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, newFlag ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	mBuffers[index].lastModifierKey = key;

	auto signature = uint32_t(vertexSize) ^ dataType;
	if (newFlag || mBuffers[index].signature != signature) {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, vertexSize, dataType, GL_FALSE, 0, nullptr);
		mBuffers[index].signature = signature;
	}
}

void gl::Vao::insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType) {
	bind();
	bool newFlag = true;
	if (mBuffers.size() <= index) {
		mBuffers.resize(index + 1);
		glGenBuffers(1, &mBuffers[index].handle);
		assert(mBuffers[index].handle);
	} else if (mBuffers[index].handle == 0)
	{
		glGenBuffers(1, &mBuffers[index].handle);
		assert(mBuffers[index].handle);
	} else
	{
		newFlag = false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, mBuffers[index].handle);
	
	// You can use the glBufferSubData function to update buffer contents, but doing so incurs a performance penalty
	// because it flushes the command buffer and waits for all commands to complete.
	//
	// Complex UI tests showed better performance on glBufferData.
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, newFlag ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	auto signature = uint32_t(vertexSize) ^ dataType;
	if (newFlag || mBuffers[index].signature != signature) {
		glEnableVertexAttribArray(index);
		glVertexAttribIPointer(index, vertexSize, dataType, GL_TRUE, 0);
		mBuffers[index].signature = signature;
	}
}

void gl::Vao::drawElements(GLenum type) {
	assert(mIndicesBuffer);
	bind();
	glDrawElements(type, mIndicesCount, GL_UNSIGNED_INT, 0);
}


void gl::Vao::indices(AArrayView<GLuint> data) {
	bind();
	GLenum drawType = GL_DYNAMIC_DRAW;
	if (mIndicesBuffer == 0) {
		glGenBuffers(1, &mIndicesBuffer);
		assert(mIndicesBuffer);
		drawType = GL_STATIC_DRAW;
	}
	mIndicesCount = static_cast<GLsizei>(data.size());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.sizeInBytes(), data.data(), drawType);
}
