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
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include <AUI/Platform/CommonRenderingContext.h>
#include "ARenderingContextOptions.h"
#include "AUI/GL/OpenGLRenderer.h"

class OpenGLRenderingContext: public CommonRenderingContext {
public:
    OpenGLRenderingContext(const ARenderingContextOptions::OpenGL& config) : mConfig(config) {}

    ~OpenGLRenderingContext() override;

    /**
     * @brief Captures what the window shows.
     * @details
     * The content is taken from the window-sized (not supersampled) buffer: the offscreen supersampled framebuffer is
     * downsampled the same way as when it is presented, and the result is read with glReadPixels. If the context
     * doesn't use an offscreen framebuffer, the default framebuffer is read.
     */
    AImage makeScreenshot() override;

#if !AUI_PLATFORM_LINUX
    // to be implemented by IPlatformAbstraction
    void init(const Init& init);
    void destroyNativeWindow(ASurface& window) override;
#endif

    void beginPaint(ASurface& window) override;
    void endPaint(ASurface& window) override;
    void beginResize(ASurface& window) override;
    void endResize(ASurface& window) override;

    [[nodiscard]]
    uint32_t getDefaultFb() const noexcept;

    void bindViewport();

    [[nodiscard]]
    glm::uvec2 viewportSize() const noexcept {
        return mViewportSize;
    }

    [[nodiscard]]
    uint32_t getSupersamplingRatio() const noexcept;

    [[nodiscard]]
    AOptional<gl::Framebuffer*> framebuffer() noexcept {
        if (auto fb = std::get_if<gl::Framebuffer>(&mFramebuffer)) {
            return fb;
        }
        return std::nullopt;
    }

    IRenderer& renderer() override {
        return *mRenderer;
    }

    static gl::Framebuffer newOffscreenRenderingFramebuffer(glm::uvec2 initialSize);

protected:
    /**
     * @brief Calls `callback` with the OpenGL context of this rendering context made current.
     * @details
     * The default implementation assumes that the context is already current on the UI thread. Platforms where it is
     * not (i.e., the context is released between frames or shared between windows) override it.
     */
    virtual void runWithGlContext(const std::function<void()>& callback);

    _<OpenGLRenderer> mRenderer;
    glm::uvec2 mViewportSize;
    struct NotTried{}; struct Failed{}; std::variant<NotTried, Failed, gl::Framebuffer> mFramebuffer;
    static _<OpenGLRenderer> ourRenderer() {
        static _weak<OpenGLRenderer> g;
        if (auto v = g.lock()) {
            return v;
        }
        auto temp = _new<OpenGLRenderer>();
        g = temp;
        return temp;
    }

    virtual void endFramebuffer();

private:
    ARenderingContextOptions::OpenGL mConfig;

    void tryEnableFramebuffer(glm::uvec2 windowSize);
    void beginFramebuffer(glm::uvec2 windowSize);

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#endif
#if AUI_PLATFORM_MACOS
    static void* ourContext;
#endif

};
