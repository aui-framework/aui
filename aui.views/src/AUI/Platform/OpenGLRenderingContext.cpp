// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "OpenGLRenderingContext.h"
#include <cstddef>
#include <functional>
#include <optional>
#include "AUI/Common/AOptional.h"
#include "AUI/GL/Framebuffer.h"
#include "AUI/GL/RenderTarget/RenderbufferRenderTarget.h"

static constexpr auto LOG_TAG = "OpenGLRenderingContext";

void OpenGLRenderingContext::tryEnableFramebuffer(glm::uvec2 windowSize) {
#if !AUI_PLATFORM_ANDROID && !AUI_PLATFORM_IOS
    if (glewIsSupported("ARB_sample_shading")) {
        ALogger::err(LOG_TAG) << "Unable to initialize multisample framebuffer: ARB_sample_shading is not present";
        mFramebuffer = Failed{};
        return;
    }
#endif
    try {
        gl::Framebuffer framebuffer;
        framebuffer.setSupersamplingRatio(2);
        framebuffer.resize(windowSize);
        auto albedo = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::RGBA8, gl::Multisampling::DISABLED>>();
        framebuffer.attach(albedo, GL_COLOR_ATTACHMENT0);
        if constexpr (true) {
            auto depth = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::DEPTH24_STENCIL8, gl::Multisampling::DISABLED>>();
            framebuffer.attach(depth, GL_DEPTH_STENCIL_ATTACHMENT /* 0x84F9*/ /* GL_DEPTH_STENCIL */);
        } else {
            auto stencil = _new<gl::RenderbufferRenderTarget<gl::InternalFormat::STENCIL8, gl::Multisampling::DISABLED>>();
            framebuffer.attach(stencil, GL_STENCIL_ATTACHMENT);
        }
        mFramebuffer.emplace<gl::Framebuffer>(std::move(framebuffer));
    } catch (const AException& e) {
        ALogger::err(LOG_TAG) << "Unable to initialize multisample framebuffer: " << e;
        mFramebuffer = Failed{};
    }
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
    mWindowSize = windowSize;
    bindViewport();
}

void OpenGLRenderingContext::endFramebuffer() {
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

AOptional<OpenGLRenderingContext::OffscreenRendering> OpenGLRenderingContext::getOffscreenRendering() {

    if (auto defaultFb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
        struct LazyHelper {
            LazyHelper(OpenGLRenderingContext& ctx, AOptional<gl::Framebuffer>& fb): ctx(ctx), fb(fb) {

            }
            std::reference_wrapper<gl::Framebuffer> operator()() const {
                if (!fb) {
                    fb.emplace();
                }
                if (fb->size() != ctx.mWindowSize) {
                    fb->resize(ctx.mWindowSize);
                }
                return *fb;
            }
            OpenGLRenderingContext& ctx;
            AOptional<gl::Framebuffer>& fb;
        };

        return OffscreenRendering {
            .defaultRenderTarget = *defaultFb,
            .renderTarget0 = LazyHelper(*this, mHelperFramebuffer0),
            .renderTarget1 = LazyHelper(*this, mHelperFramebuffer1),
            .windowSize = mWindowSize,
        };
    }
    return std::nullopt;
}
