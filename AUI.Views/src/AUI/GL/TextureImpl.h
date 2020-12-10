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
void GL::Texture<TEXTURE_TARGET>::bind(uint8_t index) {
    GL::State::activeTexture(index);
    GL::State::bindTexture(TEXTURE_TARGET, mTexture);
}

