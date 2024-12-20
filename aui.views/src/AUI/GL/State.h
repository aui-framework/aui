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

//
// Created by alex2772 on 30.07.2018.
//

#pragma once

#include <tuple>
#include <cstdint>
#include "gl.h"
#include "AUI/Views.h"


// comment this define in order to debug state machine cache failures
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


