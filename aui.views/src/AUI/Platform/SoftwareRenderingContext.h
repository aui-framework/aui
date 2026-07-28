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

class API_AUI_VIEWS SoftwareRenderingContext: public aui::noncopyable, public CommonRenderingContext {
public:
    SoftwareRenderingContext();
    ~SoftwareRenderingContext() override;

#if !AUI_PLATFORM_LINUX
    // to be implemented by IPlatformAbstraction
    void init(const Init& init) override;
    void destroyNativeWindow(ASurface& window) override;
#endif

    void beginPaint(ASurface& window) override;
    void endPaint(ASurface& window) override;
    void beginResize(ASurface& window) override;

    IRenderer& renderer() override;

    AImage makeScreenshot() override;

    inline uint8_t& stencil(const glm::uvec2& position) {
        return mStencilBlob.at<uint8_t>(mBitmapSize.x * position.y + position.x);
    }

    [[nodiscard]]
    glm::uvec2 bitmapSize() const {
        return mBitmapSize;
    }

    inline void putPixel(const glm::uvec2& position, const glm::u8vec3& color) noexcept {
        putPixel(position, glm::u8vec4(color, 255));
    }
#if AUI_PLATFORM_WIN
    inline void putPixel(const glm::uvec2& position, const glm::u8vec4& color) noexcept {
        AUI_ASSERTX(glm::all(glm::lessThan(position, mBitmapSize)), "image out of bounds");

        auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO)
            + (mBitmapSize.x * position.y + position.x) * 4);
        dataPtr[0] = color[2];
        dataPtr[1] = color[1];
        dataPtr[2] = color[0];
        dataPtr[3] = color[3];
    }
    inline glm::u8vec4 getPixel(const glm::uvec2& position) noexcept {
        AUI_ASSERTX(glm::all(glm::lessThan(position, mBitmapSize)), "image out of bounds");

        auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO)
                                                  + (mBitmapSize.x * position.y + position.x) * 4);

        return { dataPtr[2], dataPtr[1], dataPtr[0], dataPtr[3] };
    }
#else
    inline void putPixel(const glm::uvec2& position, const glm::u8vec4& color) noexcept {
        AUI_ASSERTX(glm::all(glm::lessThan(position, mBitmapSize)), "image out of bounds");

        auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob + (mBitmapSize.x * position.y + position.x) * 4);
        dataPtr[0] = color[2];
        dataPtr[1] = color[1];
        dataPtr[2] = color[0];
        dataPtr[3] = color[3];
    }
    inline glm::u8vec4 getPixel(const glm::uvec2& position) noexcept {
        AUI_ASSERTX(glm::all(glm::lessThan(position, mBitmapSize)), "image out of bounds");

        auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob + (mBitmapSize.x * position.y + position.x) * 4);
        return {
            dataPtr[2],
            dataPtr[1],
            dataPtr[0],
            dataPtr[3],
        };
    }
#endif

    void endResize(ASurface& window) override;

protected:
    AByteBuffer mStencilBlob;
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