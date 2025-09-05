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
// Created by alex2772 on 25.07.2018.
//

#include <cassert>
#include <cstring>
#include "Texture2D.h"
#include "gl.h"
#include "TextureImpl.h"
#include "TextureFormatRecognition.h"

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowBase.h>

template
class gl::Texture<gl::TEXTURE_2D>;

void gl::Texture2D::framebufferTex2D(glm::u32vec2 size, gl::Type type) {
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, static_cast<GLenum>(type), nullptr);
}


void gl::Texture2D::tex2D(AImageView image) {
    AUI_ASSERT_UI_THREAD_ONLY();
    bind();
    auto types = aui::gl::impl::recognize(image);

    if (mSize == image.size() && mSize != glm::u32vec2(0)) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), types.format, types.type,
                        image.buffer().data());
    } else {
        mSize = image.size();

        AUI_ASSERTX(image.format().bytesPerPixel() * image.width() * image.height() <= image.buffer().size() || image.buffer().empty(), "bad buffer");
        glTexImage2D(GL_TEXTURE_2D, 0, types.internalformat, image.width(), image.height(), 0, types.format, types.type,
                     image.buffer().empty() ? nullptr : image.buffer().data());
    }
}
