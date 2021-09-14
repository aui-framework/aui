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

#include "State.h"

using namespace GL;


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
