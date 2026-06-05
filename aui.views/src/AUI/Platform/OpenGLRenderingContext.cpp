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

#include "OpenGLRenderingContext.h"
#include <AUI/GL/gl.h>

static constexpr auto LOG_TAG = "OpenGLRenderingContext";

OpenGLRenderingContext::OpenGLRenderingContext(const ARenderingContextOptions::OpenGL& config) : mConfig(config) {}

void OpenGLRenderingContext::beginFramebuffer(glm::uvec2 windowSize) {
    mViewportSize = windowSize;
    if (!mWindowTarget || mWindowTarget->getSize() != mViewportSize) {
        // backbuffer fb
        mWindowTarget = mRenderer->createFramebufferWrapper(gl::Framebuffer::DEFAULT_FB, mViewportSize);
    }
}

uint32_t OpenGLRenderingContext::getDefaultFb() const noexcept {
    return gl::Framebuffer::current() ? gl::Framebuffer::current()->getHandle() : gl::Framebuffer::DEFAULT_FB;
}
