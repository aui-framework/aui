// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#pragma once

#include "Texture.h"
#include "gl.h"
#include "ResourcePool.h"
#include <AUI/GL/State.h>


template<unsigned int TEXTURE_TARGET>
gl::Texture<TEXTURE_TARGET>::Texture() {
    mTexture = gl::ResourcePool<gl::ResourceKind::TEXTURE>::get();
    gl::State::bindTexture(TEXTURE_TARGET, mTexture);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

template<unsigned int TEXTURE_TARGET>
gl::Texture<TEXTURE_TARGET>::~Texture() {
    gl::ResourcePool<gl::ResourceKind::TEXTURE>::put(mTexture);
    gl::State::bindTexture(TEXTURE_TARGET, 0);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupNearest() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupLinear() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupMirroredRepeat() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}
template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupClampToEdge() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::bind(uint8_t index) {
    gl::State::activeTexture(index);
    gl::State::bindTexture(TEXTURE_TARGET, mTexture);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::unbind(uint8_t index) {
    gl::State::activeTexture(index);
    gl::State::bindTexture(TEXTURE_TARGET, 0);
}
