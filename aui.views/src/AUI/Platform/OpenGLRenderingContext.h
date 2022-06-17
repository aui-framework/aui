//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include "CommonRenderingContext.h"
#include "ARenderingContextOptions.h"

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

private:
    ARenderingContextOptions::OpenGL mConfig;

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
    static void makeCurrent(HDC hdc) noexcept;
#elif AUI_PLATFORM_LINUX
    static GLXContext ourContext;
#elif AUI_PLATFORM_MACOS
    void* mContext;
#endif

};