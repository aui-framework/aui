#pragma once


#include "IRenderingContext.h"

/**
 * Fake window initializer which does not create any window. Applicable in UI tests
 */
class AFakeWindowInitializer: public IRenderingContext {
public:
    void init(const Init& init) override;

    ~AFakeWindowInitializer() override = default;

    void beginPaint(AWindow& window) override;

    void endPaint(AWindow& window) override;

    void beginResize(AWindow& window) override;

    void destroyNativeWindow(AWindow& window) override;
};


