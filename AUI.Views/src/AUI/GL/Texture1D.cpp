//
// Created by alex2 on 09.12.2020.
//

#include "Texture1D.h"

#include "TextureImpl.h"

template class GL::Texture<GL::TEXTURE_1D>;

void GL::Texture1D::tex1D(const AVector<AColor>& image) {
    bind();
    glTexImage1D(GL::TEXTURE_1D, 0, GL_RGBA, image.size(), 0, GL_RGBA, GL_FLOAT, image.data());
}
