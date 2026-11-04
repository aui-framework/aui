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
#include "AUI/Image/AImage.h"
#include "AUI/Util/kAUI.h"

static constexpr auto LOG_TAG = "OpenGLRenderingContext";

void OpenGLRenderingContext::tryEnableFramebuffer(glm::uvec2 windowSize) {
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
#if AUI_PLATFORM_WIN
            // Parallels' OpenGL keeps the old size of renderbuffers respecified with glRenderbufferStorage, clipping
            // everything drawn to the framebuffer. Recreating the renderbuffers works around it.
            tryEnableFramebuffer(windowSize);
#else
            fb->resize(windowSize);
#endif
        }
    }
    if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
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
namespace {
/// Reads the currently bound read framebuffer to a top-down RGBA image.
AImage readBoundFramebuffer(glm::uvec2 size) {
    AImage result(size, APixelFormat::RGBA_BYTE);
    GLint previousAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                 static_cast<void*>(result.modifiableBuffer().data()));
    glPixelStorei(GL_PACK_ALIGNMENT, previousAlignment);
    result.mirrorVertically();   // opengl origin is the bottom left corner
    return result;
}
}   // namespace

#if !AUI_PLATFORM_WIN && !AUI_PLATFORM_MACOS
void OpenGLRenderingContext::runWithGlContext(const std::function<void()>& callback) { callback(); }
#endif

AImage OpenGLRenderingContext::makeScreenshot() {
    AImage result;
    runWithGlContext([&] {
        // the framebuffer bindings are restored afterwards, so we don't confuse the renderer with our actions. note that
        // gl::Framebuffer::bind* are avoided intentionally: they update the current framebuffer tracking.
        GLint previousRead = 0, previousDraw = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousRead);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDraw);
        AUI_DEFER {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, previousRead);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousDraw);
        };

#if !AUI_PLATFORM_EMSCRIPTEN
        if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
            const auto size = fb->size();
            if (size.x == 0 || size.y == 0) {
                return;
            }
            if (fb->supersamlingRatio() <= 1) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, fb->getHandle());
                result = readBoundFramebuffer(size);
                return;
            }

            // downsample to the window resolution the same way as endFramebuffer does when presenting.
            GLuint resolveFramebuffer = 0, resolveRenderbuffer = 0;
            glGenFramebuffers(1, &resolveFramebuffer);
            glGenRenderbuffers(1, &resolveRenderbuffer);
            AUI_DEFER {
                glDeleteFramebuffers(1, &resolveFramebuffer);
                glDeleteRenderbuffers(1, &resolveRenderbuffer);
            };
            GLint previousRenderbuffer = 0;
            glGetIntegerv(GL_RENDERBUFFER_BINDING, &previousRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, resolveRenderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size.x, size.y);
            glBindRenderbuffer(GL_RENDERBUFFER, previousRenderbuffer);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFramebuffer);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolveRenderbuffer);
            if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                ALogger::err(LOG_TAG) << "Unable to make a screenshot: resolve framebuffer is incomplete";
                return;
            }
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fb->getHandle());
            const auto supersampled = fb->supersampledSize();
            glBlitFramebuffer(0, 0, supersampled.x, supersampled.y,   // src
                              0, 0, size.x, size.y,                   // dst
                              GL_COLOR_BUFFER_BIT, GL_LINEAR);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFramebuffer);
            result = readBoundFramebuffer(size);
            return;
        }
#endif

        // no offscreen framebuffer: the picture lives in the default one.
        if (mViewportSize.x == 0 || mViewportSize.y == 0) {
            return;
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl::Framebuffer::DEFAULT_FB);
        result = readBoundFramebuffer(mViewportSize);
    });
    return result;
}
