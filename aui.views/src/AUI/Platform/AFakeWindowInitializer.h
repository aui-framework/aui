#pragma once


#include "IRenderingContext.h"
#include "ABaseWindow.h"

/**
 * Fake window initializer which does not create any window. Applicable in UI tests
 */
class AFakeWindowInitializer: public IRenderingContext {
public:
    void init(const Init& init) override;

    ~AFakeWindowInitializer() override = default;

    void beginPaint(ABaseWindow& window) override;

    void endPaint(ABaseWindow& window) override;

    void beginResize(ABaseWindow& window) override;

    void destroyNativeWindow(ABaseWindow& window) override;
};


