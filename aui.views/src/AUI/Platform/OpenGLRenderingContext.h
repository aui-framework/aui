//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include "CommonRenderingContext.h"

class OpenGLRenderingContext: public CommonRenderingContext {
private:

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
#elif AUI_PLATFORM_LINUX
    static GLXContext ourContext;
#elif AUI_PLATFORM_MACOS
    void* mContext;
#endif

public:
    void init(const Init& init) override;
    ~OpenGLRenderingContext() override;

    AImage makeScreenshot() override;

    void destroyNativeWindow(AWindow& window) override;
    void beginPaint(AWindow& window) override;
    void endPaint(AWindow& window) override;
    void beginResize(AWindow& window) override;
    void endResize(AWindow& window) override;
};