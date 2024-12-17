/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/GL/Framebuffer.h>

namespace gl {
    struct API_AUI_VIEWS RenderTargetDepth {
    public:
        static std::unique_ptr<Framebuffer::IRenderTarget> attach(gl::Framebuffer& to);
    };

    struct API_AUI_VIEWS RenderTargetRGBA8 {
    public:
        static std::unique_ptr<Framebuffer::IRenderTarget> attach(gl::Framebuffer& to, GLenum attachment /* = GL_COLOR_ATTACHMENT0 */);
    };
}
