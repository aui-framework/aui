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

#include <AUI/GL/gl.h>
#include <AUI/Image/AImage.h>


namespace aui::gl::impl {

struct TextureFormat {
    /*
     * GL_R16F / GL_RGB16F / GL_RGBA16F / GL_RGBA ....
     */
    GLint internalformat = 0;

    /*
     * GL_RED / GL_RGB / GL_RGBA
     */
    GLenum format = 0;

    /*
     * GL_FLOAT / GL_UNSIGNED_BYTE
     */
    GLenum type = GL_FLOAT;

    bool operator==(const TextureFormat& rhs) const = default;
};

API_AUI_VIEWS TextureFormat recognize(AImageView image);
}
