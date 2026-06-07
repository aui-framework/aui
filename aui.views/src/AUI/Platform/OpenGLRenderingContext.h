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
#include "AUI/GL/OpenGLBackend.hpp"
#include <AUI/Render/ADrawList.hpp>
#include <AUI/Render/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>

class OpenGLRenderingContext: public CommonRenderingContext {
public:
    OpenGLRenderingContext(const ARenderingContextOptions::OpenGL& config);

    ~OpenGLRenderingContext() override;

    AImage makeScreenshot() override;

#if !AUI_PLATFORM_LINUX
    // to be implemented by IPlatformAbstraction
    void init(const Init& init) override;
    void destroyNativeWindow(ASurface& window) override;
#endif

    void beginPaint(ASurface& window) override;
    void endPaint(ASurface& window) override;
    void beginResize(ASurface& window) override;
    void endResize(ASurface& window) override;

    [[nodiscard]]
    uint32_t getDefaultFb() const noexcept;

    [[nodiscard]]
    glm::uvec2 viewportSize() const noexcept {
        return mViewportSize;
    }

    IRenderer& renderer() override {
        return *mRendererWrapper;
    }

    IRendererBackend& backend() override {
        return *mRenderer;
    }

    ACanvas& canvas() override {
        return *mCanvas;
    }

protected:
    virtual void bindContext();
    _<OpenGLBackend> mRenderer;
    ADrawList mDrawList;
    ADrawList mPresentDisplayList;
    _unique<ADisplayListCanvas> mCanvas;
    _unique<RendererCanvas> mRendererWrapper;
    glm::uvec2 mViewportSize { 0, 0 };
    _<ITexture> mWindowTarget;
    _<ITexture> mBackbufferTarget;
    bool mFlipY = false;
    static _<OpenGLBackend> ourRenderer() {
        static _weak<OpenGLBackend> g;
        if (auto v = g.lock()) {
            return v;
        }
        auto temp = _new<OpenGLBackend>();
        g = temp;
        return temp;
    }

private:
    ARenderingContextOptions::OpenGL mConfig;

    void beginFramebuffer(glm::uvec2 windowSize);
    void presentToBackbuffer();

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#endif
#if AUI_PLATFORM_MACOS
    static void* ourContext;
#endif

};
