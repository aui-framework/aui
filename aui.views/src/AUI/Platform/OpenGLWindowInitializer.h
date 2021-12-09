//
// Created by Alex2772 on 12/7/2021.
//

#pragma once


#include "CommonWindowInitializer.h"

class OpenGLWindowInitializer: public CommonWindowInitializer {
private:

#if AUI_PLATFORM_WIN

    HGLRC mHrc;
#endif

public:
    ~OpenGLWindowInitializer() override;
    void initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                          AWindow* parent) override;

    void destroyNativeWindow(AWindow& window) override;

    void beginPaint(AWindow& window) override;

    void endPaint(AWindow& window) override;

    void beginResize(AWindow& window) override;

};