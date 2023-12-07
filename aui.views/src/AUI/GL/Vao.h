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

		void bind() const noexcept;
		static void unbind() noexcept;

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
