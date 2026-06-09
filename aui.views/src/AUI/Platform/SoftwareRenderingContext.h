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

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Render/ARender/ADrawList.hpp>
#include <AUI/Render/ARender/ADisplayListCanvas.hpp>
#include <AUI/Render/RendererCanvas.h>

class SoftwareRenderer;

class API_AUI_VIEWS SoftwareRenderingContext: public aui::noncopyable, public CommonRenderingContext {
public:
    SoftwareRenderingContext();
    ~SoftwareRenderingContext() override;

    void init(const Init& init) override;
#if !AUI_PLATFORM_LINUX
    // to be implemented by IPlatformAbstraction
    void destroyNativeWindow(ASurface& window) override;
#endif

    void beginPaint(ASurface& window) override;
    void endPaint(ASurface& window) override;
    void beginResize(ASurface& window) override;

    IRenderer& renderer() override {
        return *mRendererWrapper;
    }

    IRendererBackend& backend() override;

    ACanvas& canvas() override {
        return *mCanvas;
    }

    AImage makeScreenshot() override;

    [[nodiscard]]
    glm::uvec2 bitmapSize() const {
        return mBitmapSize;
    }

    [[nodiscard]]
    const glm::uvec2& getBitmapSize() const noexcept {
        return mBitmapSize;
    }

    void endResize(ASurface& window) override;

protected:
    _<SoftwareRenderer> mRenderer;
    ADrawList mDrawList;
    _unique<ADisplayListCanvas> mCanvas;
    _unique<RendererCanvas> mRendererWrapper;

    _<ITexture> mWindowTarget;
    glm::uvec2 mBitmapSize;
#if AUI_PLATFORM_LINUX
    std::uint8_t* mBitmapBlob = nullptr;
#endif

    void reallocate(const ASurface& window);
    virtual void reallocate();

private:
#if AUI_PLATFORM_WIN
    AByteBuffer mBitmapBlob;
    BITMAPINFO* mBitmapInfo;
#endif
#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_APPLE || AUI_PLATFORM_EMSCRIPTEN
    std::uint8_t* mBitmapBlob = nullptr;
#endif
};