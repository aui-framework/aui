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
