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
// Created by alex2772 on 30.07.2018.
//

#pragma once

#include <tuple>
#include <cstdint>
#include "gl.h"
#include "AUI/Views.h"

#define SO_STATE

namespace GL {
	/**
	 * \brief Helper class for optimizing gl calls count
	 */
	class API_AUI_VIEWS State {
	private:
		State();
		~State();
		State(const State&) = delete;
		template <int K, typename F, typename... Args>
		inline static void impl(F functor, Args... args) {
#ifdef SO_STATE
			static std::tuple<Args...> prevState;
			std::tuple<Args...> state(args...);
			if (prevState != state) {
				prevState = state;
				functor(args...);
			}
#else
			functor(args...);
#endif
		};
	public:
		static void bindTexture(GLenum mode, GLuint texture);
		static void activeTexture(uint8_t index);
		static void useProgram(GLuint program);
		static void bindFramebuffer(GLenum type, GLuint framebuffer);
		static void bindVertexArray(GLuint handle);
	};
}


