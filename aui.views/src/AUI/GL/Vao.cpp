/**
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

#include "Vao.h"
#include "State.h"



GL::Vao::Vao() {
    glGenVertexArrays(1, &mHandle);
	bind();
}

GL::Vao::~Vao() {
	auto buffers = mBuffers;
	auto handle = mHandle;

	GL::State::bindVertexArray(handle);
	glDeleteBuffers(1, &mIndicesBuffer);
	glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
	glDeleteVertexArrays(1, &handle);
	GL::State::bindVertexArray(0);
}

const AVector<GLuint>& GL::Vao::getBuffers() const
{
	return mBuffers;
}

void GL::Vao::bind() {
	GL::State::bindVertexArray(mHandle);
}


void GL::Vao::draw(GLenum type, GLsizei count) {
	bind();
	glDrawArrays(type, 0, count);
}

void GL::Vao::insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType) {
	bind();
	bool newFlag = true;
	if (mBuffers.size() <= index) {
		mBuffers.resize(index + 1);
		glGenBuffers(1, &mBuffers[index]);
		assert(mBuffers[index]);
	} else if (mBuffers[index] == 0)
	{
		glGenBuffers(1, &mBuffers[index]);
		assert(mBuffers[index]);
	} else
	{
		newFlag = false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, mBuffers[index]);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, newFlag ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

	if (newFlag) {
		glEnableVertexAttribArray(index);
	}
    glVertexAttribPointer(index, vertexSize, dataType, GL_TRUE, 0, nullptr);
}

void GL::Vao::insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType) {
	GLuint buffer;
	bind();
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, vertexSize, dataType, 0, nullptr);
	mBuffers.push_back(buffer);
}

void GL::Vao::insert(GLuint index, const AVector<float>& data) {
	insert(index, (const char*)data.data(), data.size() * sizeof(float), 1, GL_FLOAT);
}
void GL::Vao::insert(GLuint index, const AVector<glm::vec2>& data) {
	insert(index, (const char*)data.data(), data.size() * sizeof(glm::vec2), 2, GL_FLOAT);
}

void GL::Vao::insert(GLuint index, const AVector<glm::vec3>& data) {
	insert(index, (const char*)data.data(), data.size() * sizeof(glm::vec3), 3, GL_FLOAT);
}

void GL::Vao::insert(GLuint index, const AVector<glm::vec4>& data) {
	insert(index, (const char*)data.data(), data.size() * sizeof(glm::vec4), 4, GL_FLOAT);
}
void GL::Vao::insert(GLuint index, const AVector<GLuint>& data) {
	insertInteger(index, (const char*)data.data(), data.size() * sizeof(GLuint), 1, GL_UNSIGNED_INT);
}

void GL::Vao::draw(GLenum type) {
	assert(mIndicesBuffer);
	bind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
 	glDrawElements(type, mIndicesCount, GL_UNSIGNED_INT, 0);
}

void GL::Vao::indices(const AVector<GLuint>& data) {
	GLenum drawType = GL_DYNAMIC_DRAW;
	if (mIndicesBuffer == 0) {
		glGenBuffers(1, &mIndicesBuffer);
		assert(mIndicesBuffer);
		drawType = GL_STATIC_DRAW;
	}
	mIndicesCount = static_cast<GLsizei>(data.size());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(GLuint), data.data(), drawType);
}
