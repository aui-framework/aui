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
