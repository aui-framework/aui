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

//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include <AUI/Platform/CommonRenderingContext.h>
#include "ARenderingContextOptions.h"
#include "AUI/Common/AOptional.h"
#include "AUI/GL/Framebuffer.h"
#include "AUI/GL/OpenGLRenderer.h"

class OpenGLRenderingContext: public CommonRenderingContext {
public:
    OpenGLRenderingContext(const ARenderingContextOptions::OpenGL& config) : mConfig(config) {}

    void init(const Init& init) override;
    ~OpenGLRenderingContext() override;

    AImage makeScreenshot() override;

    void destroyNativeWindow(ABaseWindow& window) override;
    void beginPaint(ABaseWindow& window) override;
    void endPaint(ABaseWindow& window) override;
    void beginResize(ABaseWindow& window) override;
    void endResize(ABaseWindow& window) override;

    [[nodiscard]]
    uint32_t getDefaultFb() const noexcept;

    void bindViewport();

    [[nodiscard]]
    glm::uvec2 viewportSize() const noexcept {
        return mViewportSize;
    }

    [[nodiscard]]
    uint32_t getSupersamplingRatio() const noexcept;

    struct OffscreenRendering {
        /**
         * @brief Default render target where AUI normally renders to. Typically twice as big as the window size.
         */
        gl::Framebuffer& defaultRenderTarget;


        /**
         * @brief Helper render target. Typically matches window size.
         */
        aui::lazy<std::reference_wrapper<gl::Framebuffer>> renderTarget0;

        /**
         * @brief Helper render target. Typically matches window size.
         */
        aui::lazy<std::reference_wrapper<gl::Framebuffer>> renderTarget1;

        glm::uvec2 windowSize;
    };

    AOptional<OffscreenRendering> getOffscreenRendering();

   private:
    ARenderingContextOptions::OpenGL mConfig;
    struct NotTried{}; struct Failed{}; std::variant<NotTried, Failed, gl::Framebuffer> mFramebuffer;
    AOptional<gl::Framebuffer> mHelperFramebuffer0, mHelperFramebuffer1; // mapped to OffscreenRendering
    _<OpenGLRenderer> mRenderer;
    glm::uvec2 mViewportSize, mWindowSize;

    static _<OpenGLRenderer> ourRenderer() {
        static _weak<OpenGLRenderer> g;
        if (auto v = g.lock()) {
            return v;
        }
        auto temp = _new<OpenGLRenderer>();
        g = temp;
        return temp;
    }

    void tryEnableFramebuffer(glm::uvec2 windowSize);
    void beginFramebuffer(glm::uvec2 windowSize);
    void endFramebuffer();

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#elif AUI_PLATFORM_LINUX
    static GLXContext ourContext;
#elif AUI_PLATFORM_MACOS
    void* mContext;
#endif

};
