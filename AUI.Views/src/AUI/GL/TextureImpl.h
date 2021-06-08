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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include "Texture.h"
#include "gl.h"
#include <AUI/GL/State.h>


template<unsigned int TEXTURE_TARGET>
GL::Texture<TEXTURE_TARGET>::Texture() {
    glGenTextures(1, &mTexture);
    GL::State::bindTexture(TEXTURE_TARGET, mTexture);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

template<unsigned int TEXTURE_TARGET>
GL::Texture<TEXTURE_TARGET>::~Texture() {
    glDeleteTextures(1, &mTexture);
    GL::State::bindTexture(TEXTURE_TARGET, 0);
}

template<unsigned int TEXTURE_TARGET>
void GL::Texture<TEXTURE_TARGET>::setupNearest() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
template<unsigned int TEXTURE_TARGET>
void GL::Texture<TEXTURE_TARGET>::setupLinear() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

template<unsigned int TEXTURE_TARGET>
void GL::Texture<TEXTURE_TARGET>::setupMirroredRepeat() {
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}

template<unsigned int TEXTURE_TARGET>
void GL::Texture<TEXTURE_TARGET>::bind(uint8_t index) {
    GL::State::activeTexture(index);
    GL::State::bindTexture(TEXTURE_TARGET, mTexture);
}

