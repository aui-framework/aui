//
// Created by Alex2772 on 11/22/2021.
//

#include "Vbo.h"

GL::detail::VboImpl::VboImpl() {
    glGenBuffers(1, &mHandle);
}

GL::detail::VboImpl::~VboImpl() {
    if (mHandle != 0) {
        glDeleteBuffers(1, &mHandle);
    }
}

void GL::VertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
}

void GL::VertexBuffer::insert(const char* data, size_t length) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
}


void GL::IndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
}

void GL::IndexBuffer::set(const GLuint* indices, GLsizei length) {
    bind();
    mIndicesCount = length / sizeof(GLuint);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, indices, GL_STATIC_DRAW);
}
