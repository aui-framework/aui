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

#pragma once

#include "Texture.h"
#include "gl.h"
#include "ResourcePool.h"
#include <AUI/GL/State.h>


template<unsigned int TEXTURE_TARGET>
gl::Texture<TEXTURE_TARGET>::Texture() {
    mTexture = gl::ResourcePool<gl::ResourceKind::TEXTURE>::get();
    gl::State::bindTexture(TEXTURE_TARGET, mTexture);
    setupLinear();
    setupMirroredRepeat();
}

template<unsigned int TEXTURE_TARGET>
gl::Texture<TEXTURE_TARGET>::~Texture() {
    if (mTexture == 0) {
        return;
    }
    gl::ResourcePool<gl::ResourceKind::TEXTURE>::put(mTexture);
    gl::State::bindTexture(TEXTURE_TARGET, 0);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupNearest() {
    if (mFiltering == Filtering::NEAREST) {
        return;
    }
    bind();
    mFiltering = Filtering::NEAREST;
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupLinear() {
    if (mFiltering == Filtering::LINEAR) {
        return;
    }
    bind();
    mFiltering = Filtering::LINEAR;
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupRepeat() {
    if (mWrapping == Wrapping::REPEAT) {
        return;
    }
    bind();
    mWrapping = Wrapping::REPEAT;
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupMirroredRepeat() {
    if (mWrapping == Wrapping::MIRRORED_REPEAT) {
        return;
    }
    bind();
    mWrapping = Wrapping::MIRRORED_REPEAT;
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::setupClampToEdge() {
    if (mWrapping == Wrapping::CLAMP_TO_EDGE) {
        return;
    }
    bind();
    mWrapping = Wrapping::CLAMP_TO_EDGE;
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::bind(uint8_t index) {
    AUI_ASSERTX(getHandle() != 0, "gl::Texture::bind: mTexture == 0");
    gl::State::activeTexture(index);
    gl::State::bindTexture(TEXTURE_TARGET, mTexture);
}

template<unsigned int TEXTURE_TARGET>
void gl::Texture<TEXTURE_TARGET>::unbind(uint8_t index) {
    gl::State::activeTexture(index);
    gl::State::bindTexture(TEXTURE_TARGET, 0);
}
