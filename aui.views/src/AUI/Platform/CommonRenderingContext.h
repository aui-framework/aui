#pragma once

#include "IRenderingContext.h"
#include "AWindow.h"

class CommonRenderingContext: public IRenderingContext {
protected:
#if AUI_PLATFORM_WIN

    AString mWindowClass;

    /**
     * GetDC() HDC
     */
    HDC mWindowDC;

    /**
     * BeginPaint() HDC
     */
    HDC mPainterDC;

    PAINTSTRUCT mPaintstruct;

#endif
public:
    void init(const Init& init) override;

    void beginPaint(AWindow& window) override;

    void endPaint(AWindow& window) override;

    ~CommonRenderingContext() override = default;

    void destroyNativeWindow(AWindow& window) override;
};