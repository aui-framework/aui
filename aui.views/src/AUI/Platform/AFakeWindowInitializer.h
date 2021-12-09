#pragma once


#include "IWindowInitializer.h"

/**
 * Fake window initializer which does not create any window. Applicable in UI tests
 */
class AFakeWindowInitializer: public IWindowInitializer {
public:
    void initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                          AWindow* parent) override;

    void destroyNativeWindow(AWindow& window) override;
};


