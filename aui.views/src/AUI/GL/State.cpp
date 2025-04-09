/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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
