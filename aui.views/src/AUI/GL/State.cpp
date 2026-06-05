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

namespace {
    uint8_t gActiveTextureUnit = 0;
}

void State::activeTexture(uint8_t index) {
#ifdef SO_STATE
	static uint8_t prevIndex = 0xff;
	if (prevIndex != index) {
		prevIndex = index;
		glActiveTexture(GL_TEXTURE0 + index);
	}
#else
	glActiveTexture(GL_TEXTURE0 + index);
#endif
	gActiveTextureUnit = index;
}

void State::bindTexture(GLenum mode, GLuint texture) {
#ifdef SO_STATE
	static std::tuple<uint8_t, GLenum, GLuint> prevState = {0xff, 0, 0};
	std::tuple<uint8_t, GLenum, GLuint> state = {gActiveTextureUnit, mode, texture};
	if (prevState != state) {
		prevState = state;
		glBindTexture(mode, texture);
	}
#else
	glBindTexture(mode, texture);
#endif
}

void State::useProgram(GLuint program) {
	impl<2>(glUseProgram, program);
}

void State::bindFramebuffer(GLenum type, GLuint framebuffer) {
	impl<3>(glBindFramebuffer, type, framebuffer);
}

void State::bindVertexArray(GLuint handle) {
	glBindVertexArray(handle);
}

void State::label(GLenum identifier, GLuint name, const AString& labelText) {
	if (glObjectLabel) {
		std::string s = labelText.toStdString();
		glObjectLabel(identifier, name, static_cast<GLsizei>(s.size()), s.data());
	}
}
