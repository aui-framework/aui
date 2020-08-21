//
// Created by Алексей on 30.07.2018.
//

#pragma once

#include <tuple>
#include <cstdint>
#include "gl.h"
#include "AUI/Views.h"

#define SO_STATE

namespace GL {
	/*!
	 * Класс для оптимизации количества вызовов gl
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


