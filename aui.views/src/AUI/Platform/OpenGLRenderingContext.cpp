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
#include "AUI/GL/RenderTarget/RenderbufferRenderTarget.h"

static constexpr auto LOG_TAG = "OpenGLRenderingContext";

void OpenGLRenderingContext::tryEnableFramebuffer(glm::uvec2 windowSize) {
#if !AUI_PLATFORM_ANDROID && !AUI_PLATFORM_IOS
    if (!GLAD_GL_ARB_sample_shading) {
        ALogger::err(LOG_TAG) << "Unable to initialize multisample framebuffer: ARB_sample_shading is not present";
        mFramebuffer = Failed{};
        return;
    }
#endif
    try {
        mFramebuffer.emplace<gl::Framebuffer>(newOffscreenRenderingFramebuffer(windowSize));
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to initialize multisample framebuffer: " << e;
        mFramebuffer = Failed{};
    }
}

gl::Framebuffer OpenGLRenderingContext::newOffscreenRenderingFramebuffer(glm::uvec2 initialSize) {
    gl::Framebuffer framebuffer;
    framebuffer.setSupersamplingRatio(2);
    framebuffer.resize(initialSize);
    auto albedo = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::RGBA8, gl::Multisampling::DISABLED>>();
    framebuffer.attach(albedo, GL_COLOR_ATTACHMENT0);
    auto depth = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::DEPTH24_STENCIL8, gl::Multisampling::DISABLED>>();
    framebuffer.attach(depth, GL_DEPTH_STENCIL_ATTACHMENT /* 0x84F9*/ /* GL_DEPTH_STENCIL */);
    return framebuffer;
}

void OpenGLRenderingContext::beginFramebuffer(glm::uvec2 windowSize) {
    if (std::get_if<NotTried>(&mFramebuffer)) {
        tryEnableFramebuffer(windowSize);
    }
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        if (fb->size() != windowSize) {
            fb->resize(windowSize);
        }
        fb->bind();
        mViewportSize = fb->supersampledSize();
    } else {
        mViewportSize = windowSize;
    }
    bindViewport();
}

void OpenGLRenderingContext::endFramebuffer() {
#if !AUI_PLATFORM_EMSCRIPTEN
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        fb->bindForRead();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl::Framebuffer::DEFAULT_FB);
        glBlitFramebuffer(0, 0,                       // src pos
                          fb->supersampledSize().x, fb->supersampledSize().y, // src size
                          0, 0,                       // dst pos
                          fb->size().x, fb->size().y, // dst size
                          GL_COLOR_BUFFER_BIT,        // mask
                          GL_LINEAR);                // filter
        gl::Framebuffer::unbind();
    }
#endif
}

void OpenGLRenderingContext::bindViewport() {
    glViewport(0, 0, mViewportSize.x, mViewportSize.y);
}

uint32_t OpenGLRenderingContext::getDefaultFb() const noexcept {
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        return fb->getHandle();
    }
    return 0;
}

uint32_t OpenGLRenderingContext::getSupersamplingRatio() const noexcept {
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        return fb->supersamlingRatio();
    }
    return 1;
}