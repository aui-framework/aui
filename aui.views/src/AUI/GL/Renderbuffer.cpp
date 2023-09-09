//
// Created by alex2772 on 7/17/23.
//

#include "Renderbuffer.h"

gl::Renderbuffer::Renderbuffer() {
    glGenRenderbuffers(1, &mHandle);
}

gl::Renderbuffer::~Renderbuffer() {
    glDeleteRenderbuffers(1, &mHandle);
}

void gl::Renderbuffer::bind() {
    glBindRenderbuffer(GL_RENDERBUFFER, mHandle);
}

void gl::Renderbuffer::storage(glm::u32vec2 size, gl::InternalFormat internalFormat) {
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER, (int)internalFormat, size.x, size.y);
}
