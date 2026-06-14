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


#include <AUI/Util/kAUI.h>
#include <AUI/GL/Framebuffer.h>
#include <AUI/GL/GLEnums.h>
#include <AUI/GL/Texture2D.h>

namespace gl {

    template<InternalFormat internalFormat /* = GL_DEPTH_COMPONENT24 */,
             Type type                     /* = GL_FLOAT */,
             Format format                    = Format::DEPTH_COMPONENT>
    class TextureRenderTarget final: public Framebuffer::IRenderTarget {
    public:
        TextureRenderTarget() {
        }

        void bindAsTexture(int index) {
            mTexture.bind(index);
        }

        void attach(Framebuffer& to, GLenum attachmentType) override {
            to.bind();
            glGetError();
            onFramebufferResize(to.size());
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, mTexture.getHandle(), 0);
            if (glGetError() != GL_NO_ERROR) {
                throw AException("unable to create texture target");
            }
        }

        [[nodiscard]]
        Texture2D& texture() noexcept {
            return mTexture;
        }

        void onFramebufferResize(glm::u32vec2 size) override {
            mTexture.framebufferTex2D(size, type);
        }

    private:
        Texture2D mTexture;
    };
}
