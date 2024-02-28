// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "State.h"

using namespace gl;


State::State() {

}

State::~State() {

}

void State::activeTexture(uint8_t index) {
	impl<0>(glActiveTexture, GL_TEXTURE0 + index);
}

void State::bindTexture(GLenum mode, GLuint texture) {
	impl<1>(glBindTexture, mode, texture);
}

void State::useProgram(GLuint program) {
	impl<2>(glUseProgram, program);
}

void State::bindFramebuffer(GLenum type, GLuint framebuffer) {
	impl<3>(glBindFramebuffer, type, framebuffer);
}

void State::bindVertexArray(GLuint handle) {
	impl<4>(glBindVertexArray, handle);
}
