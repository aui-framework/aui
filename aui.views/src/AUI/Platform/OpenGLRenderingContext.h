//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include "CommonRenderingContext.h"

class OpenGLRenderingContext: public CommonRenderingContext {
private:

#if AUI_PLATFORM_WIN
    static HGLRC ourHrc;
#endif

public:
    void init(const Init& init) override;
    ~OpenGLRenderingContext() override;

    void destroyNativeWindow(AWindow& window) override;

    void beginPaint(AWindow& window) override;

    void endPaint(AWindow& window) override;

    void beginResize(AWindow& window) override;

    void endResize(AWindow& window) override;

};