// AUI Framework - Declarative UI toolkit for modern C++17
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

//
// Created by alex2 on 09.12.2020.
//

#include "Texture1D.h"

#include "TextureImpl.h"

template class gl::Texture<gl::TEXTURE_1D>;

void gl::Texture1D::tex1D(const AVector<AColor>& image) {
    bind();
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_IOS
    glTexImage2D(gl::TEXTURE_2D, 0, GL_RGBA, image.size(), 1, 0, GL_RGBA, GL_FLOAT, image.data());
#else
    glTexImage1D(gl::TEXTURE_1D, 0, GL_RGBA, image.size(), 0, GL_RGBA, GL_FLOAT, image.data());
#endif
}
