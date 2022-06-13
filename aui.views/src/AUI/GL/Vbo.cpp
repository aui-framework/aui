//
// Created by Alex2772 on 11/22/2021.
//

#include "Vbo.h"

template<gl::ResourceKind T>
gl::detail::VboImpl<T>::VboImpl() {
    mHandle = gl::ResourcePool<T>::get();
}

template<gl::ResourceKind T>
gl::detail::VboImpl<T>::~VboImpl() {
    if (mHandle) gl::ResourcePool<T>::put(mHandle);
}

void gl::VertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
}

void gl::VertexBuffer::insert(const char* data, size_t length) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
}


void gl::IndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
}

void gl::IndexBuffer::set(const GLuint* indices, GLsizei length) {
    bind();
    mIndicesCount = length / sizeof(GLuint);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, length, indices, GL_STATIC_DRAW);
}
