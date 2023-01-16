// AUI Framework - Declarative UI toolkit for modern C++17
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
	glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
	glDeleteVertexArrays(1, &handle);
	gl::State::bindVertexArray(0);
}

const AVector<GLuint>& gl::Vao::getBuffers() const
{
	return mBuffers;
}

void gl::Vao::bind() {
	gl::State::bindVertexArray(mHandle);
}


void gl::Vao::drawArrays(GLenum type, GLsizei count) {
	bind();
	glDrawArrays(type, 0, count);
}

void gl::Vao::insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType) {
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

void gl::Vao::insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType) {
	GLuint buffer;
	bind();
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, vertexSize, dataType, 0, nullptr);
	mBuffers.push_back(buffer);
}

void gl::Vao::insert(GLuint index, AArrayView<float> data) {
	insert(index, (const char*)data.data(), data.sizeInBytes(), 1, GL_FLOAT);
}
void gl::Vao::insert(GLuint index, AArrayView<glm::vec2> data) {
	insert(index, (const char*)data.data(), data.sizeInBytes(), 2, GL_FLOAT);
}

void gl::Vao::insert(GLuint index, AArrayView<glm::vec3> data) {
	insert(index, (const char*)data.data(), data.sizeInBytes(), 3, GL_FLOAT);
}

void gl::Vao::insert(GLuint index, AArrayView<glm::vec4> data) {
	insert(index, (const char*)data.data(), data.sizeInBytes(), 4, GL_FLOAT);
}
void gl::Vao::insert(GLuint index, AArrayView<GLuint> data) {
	insertInteger(index, (const char*)data.data(), data.sizeInBytes(), 1, GL_UNSIGNED_INT);
}

void gl::Vao::drawElements(GLenum type) {
	assert(mIndicesBuffer);
	bind();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndicesBuffer);
 	glDrawElements(type, mIndicesCount, GL_UNSIGNED_INT, 0);
}


void gl::Vao::indices(AArrayView<GLuint> data) {
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
