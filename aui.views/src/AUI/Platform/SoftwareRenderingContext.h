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

#pragma once

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Platform/AWindow.h>

class API_AUI_VIEWS SoftwareRenderingContext: public aui::noncopyable, public CommonRenderingContext {
public:
    SoftwareRenderingContext();
    ~SoftwareRenderingContext() override;

    void destroyNativeWindow(ABaseWindow& window) override;
    void beginPaint(ABaseWindow& window) override;
    void endPaint(ABaseWindow& window) override;
    void beginResize(ABaseWindow& window) override;
    void init(const Init& init) override;

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

    void endResize(ABaseWindow& window) override;

protected:
    AByteBuffer mStencilBlob;
    glm::uvec2 mBitmapSize;

    void reallocateImageBuffers(const ABaseWindow& window);

private:
#if AUI_PLATFORM_WIN
    AByteBuffer mBitmapBlob;
    BITMAPINFO* mBitmapInfo;
#elif AUI_PLATFORM_LINUX
    std::uint8_t* mBitmapBlob = nullptr;
    _<XImage> mXImage;
    std::unique_ptr<_XGC, void(*)(GC)> mGC = {nullptr, nullptr};

    void reallocate();
#else
    std::uint8_t* mBitmapBlob = nullptr;
#endif
};