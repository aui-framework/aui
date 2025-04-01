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
