// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
// Created by alex2772 on 30.07.2018.
//

#pragma once

#include <tuple>
#include <cstdint>
#include "gl.h"
#include "AUI/Views.h"

#define SO_STATE

namespace gl {
	/**
	 * @brief Helper class for optimizing gl calls count
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


