#pragma once

#include "CommonRenderingContext.h"

class SoftwareRenderingContext: public CommonRenderingContext {
private:
    AImage mBuffer;

public:
    SoftwareRenderingContext();
    ~SoftwareRenderingContext() override;


    void destroyNativeWindow(AWindow& window) override;

    void beginPaint(AWindow& window) override;

    void endPaint(AWindow& window) override;

    void beginResize(AWindow& window) override;

    void init(const Init& init) override;
};