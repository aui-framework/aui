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

#pragma once

#include "AUI/GL/gl.h"
#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"
#include "AUI/Util/AArrayView.h"
#include "AUI/Views.h"

namespace gl {
	class API_AUI_VIEWS Vao {
	private:
		GLuint mHandle;
		AVector<GLuint> mBuffers;
		GLuint mIndicesBuffer = 0;
		GLsizei mIndicesCount = 0;



        /**
         * @brief Creates VBO
         * @param index index in VAO
         * @param data vertex data
         * @param dataSize vertex data size in bytes
         * @param vertexSize count of floats per vertex
         */
		void insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType);


        /**
         * @brief Creates integer VBO
         * @param index index in VAO
         * @param data vertex data
         * @param dataSize vertex data size in bytes
         * @param vertexSize count of integers per vertex
         */
		void insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType);

	public:
		Vao();
		~Vao();
		Vao(const Vao&) = delete;


		const AVector<GLuint>& getBuffers() const;

		void bind();

		void insert(GLuint index, AArrayView<float> data);
		void insert(GLuint index, AArrayView<glm::vec2> data);
		void insert(GLuint index, AArrayView<glm::vec3> data);
		void insert(GLuint index, AArrayView<glm::vec4> data);
		void insert(GLuint index, AArrayView<GLuint> data);

        /**
         * @brief Uploads VBO indices
         * @param data indices
         */
		void indices(AArrayView<GLuint> data);

		void drawArrays(GLenum type, GLsizei count);

        /**
         * @brief Draws buffer. Don't forget to upload indices with <code>indices</code> function/
         * @param type Primitive type
         */
		void drawElements(GLenum type = GL_TRIANGLES);
	};
}
