/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "AUI/GL/gl.h"
#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"
#include "AUI/Util/AArrayView.h"
#include "AUI/Views.h"

namespace gl {
	class API_AUI_VIEWS Vao {
	public:
		struct Buffer {
			GLuint handle = 0;
			uint32_t signature = 0; // signature used for optimization

			/**
			 * @brief String key of the last object who has modified the buffer
			 * @details
			 * The string key is used to avoid excess OpenGL buffer modifications by comparing the pointer
			 * lastModifiedKey and the new one passed to insertIfKeyMismatches. The string stored on this address is
			 * have not, but is expected to live during the whole program lifetime (use const char* string literal
			 * constants). The string value can be used for debug purposes.
			 */
			const char* lastModifierKey = nullptr;
		};

		Vao();
		~Vao();
		Vao(const Vao&) = delete;


		[[nodiscard]]
		const AVector<Buffer>& getBuffers() const noexcept {
			return mBuffers;
		}

		void bind() const noexcept;
		static void unbind() noexcept;

        /**
         * @brief Creates VBO or modifies existing one
         * @param index index in VAO
         * @param data vertex data
         * @param key see gl::Vao::Buffer::lastModifierKey
         */
		template<typename T>
		void insert(GLuint index, AArrayView<T> data, const char* key) {
			if constexpr (std::is_same_v<T, GLuint>) {
				insertInteger(index, (const char*)data.data(), data.sizeInBytes(), 1, GL_UNSIGNED_INT);
			} else {
				insert(index, (const char*)data.data(), data.sizeInBytes(), sizeof(T) / sizeof(float), GL_FLOAT, key);
			}
		}

        /**
         * @brief Creates VBO or modifies existing one, if the key pointer mismatches
         * @param index index in VAO
         * @param data vertex data
         * @param key see gl::Vao::Buffer::lastModifierKey
         */
		template<typename T>
		void insertIfKeyMismatches(GLuint index, AArrayView<T> data, const char* key) {
			insertIfKeyMismatches(index, (const char*)data.data(), data.sizeInBytes(), sizeof(T) / sizeof(float), GL_FLOAT, key);
		}

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

	private:
		GLuint mHandle;
		AVector<Buffer> mBuffers;
		GLuint mIndicesBuffer = 0;
		GLsizei mIndicesCount = 0;


        /**
         * @brief Creates float point VBO or modifies existing one
         * @param index index in VAO
         * @param data vertex data
         * @param dataSize vertex data size in bytes
         * @param vertexSize count of floats per vertex
         * @param dataType GL data type, i.e, GL_FLOAT
         * @param key see gl::Vao::Buffer::lastModifierKey
         */
		void insert(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType, const char* key);

        /**
         * @brief Creates float point VBO or modifies existing one, if the key pointer mismatches
         * @param index index in VAO
         * @param data vertex data
         * @param dataSize vertex data size in bytes
         * @param vertexSize count of floats per vertex
         * @param dataType GL data type, i.e, GL_FLOAT
         * @param key see gl::Vao::Buffer::lastModifierKey
         */
		void insertIfKeyMismatches(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType, const char* key);


        /**
         * @brief Creates integer VBO or modifies existing one
         * @param index index in VAO
         * @param data vertex data
         * @param dataSize vertex data size in bytes
         * @param vertexSize count of integers per vertex
         * @param dataType GL data type, i.e, GL_FLOAT
         */
		void insertInteger(GLuint index, const char* data, GLsizeiptr dataSize, GLuint vertexSize, GLenum dataType);
	};
}
