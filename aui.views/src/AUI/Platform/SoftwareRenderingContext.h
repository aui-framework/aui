#pragma once

#include "CommonRenderingContext.h"
#include <AUI/Platform/AWindow.h>

class SoftwareRenderingContext: public CommonRenderingContext {
private:
#ifdef AUI_PLATFORM_WIN
    AByteBuffer mBitmapBlob;
    BITMAPINFO* mBitmapInfo;
#endif
    glm::uvec2 mBitmapSize;

public:
    SoftwareRenderingContext();
    ~SoftwareRenderingContext() override;

    void destroyNativeWindow(AWindow& window) override;
    void beginPaint(AWindow& window) override;
    void endPaint(AWindow& window) override;
    void beginResize(AWindow& window) override;
    void init(const Init& init) override;

    inline void putPixel(const glm::uvec2& position, const glm::u8vec3& color) {
        if (glm::all(glm::lessThan(position, mBitmapSize))) {
            auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO)
                + (mBitmapSize.x * position.y + position.x) * 4);
            dataPtr[0] = color[2];
            dataPtr[1] = color[1];
            dataPtr[2] = color[0];
            dataPtr[3] = 255;
        }
    }
    inline glm::u8vec3 getPixel(const glm::uvec2& position) {
        if (glm::all(glm::lessThan(position, mBitmapSize))) {
            auto dataPtr = reinterpret_cast<uint8_t*>(mBitmapBlob.data() + sizeof(BITMAPINFO)
                + (mBitmapSize.x * position.y + position.x) * 4);

            return { dataPtr[2], dataPtr[1], dataPtr[0] };
        }
        return { 0, 0, 0 };
    }

    void endResize(AWindow& window) override;
};