#pragma once

#include "AUI/GL/gl.h"
#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"
#include "AUI/Views.h"

namespace GL {
	class API_AUI_VIEWS Vao {
	private:
		GLuint mHandle;
		AVector<GLuint> mBuffers;
		GLuint mIndicesBuffer = 0;
		GLsizei mIndicesCount = 0;



        /**
         * \brief Creates VBO
         * \param index index in VAO
         * \param data vertex data
         * \param dataSize vertex data size in bytes
         * \param vertexSize count of floats per vertex
         */
		void insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType);


        /**
         * \brief Creates integer VBO
         * \param index index in VAO
         * \param data vertex data
         * \param dataSize vertex data size in bytes
         * \param vertexSize count of integers per vertex
         */
		void insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType);

	public:
		Vao();
		~Vao();
		Vao(const Vao&) = delete;


		const AVector<GLuint>& getBuffers() const;

		void bind();

		void insert(GLuint index, const AVector<float>& data);
		void insert(GLuint index, const AVector<glm::vec2>& data);
		void insert(GLuint index, const AVector<glm::vec3>& data);
		void insert(GLuint index, const AVector<glm::vec4>& data);
		void insert(GLuint index, const AVector<GLuint>& data);

        /**
         * \brief Uploads VBO indices
         * \param data indices
         */
		void indices(const AVector<GLuint>& data);

		void draw(GLenum type, GLsizei count);

        /**
         * \brief Draws buffer. Don't forget to upload indices with <code>indices</code> function/
         * \param type Primitive type
         */
		void draw(GLenum type = GL_TRIANGLES);
	};
}
