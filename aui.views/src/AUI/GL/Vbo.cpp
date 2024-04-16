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

template class gl::detail::VboImpl<gl::ResourceKind::VERTEX_BUFFER>;
template class gl::detail::VboImpl<gl::ResourceKind::INDEX_BUFFER>;

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
