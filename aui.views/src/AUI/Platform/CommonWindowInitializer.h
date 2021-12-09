#pragma once

#include "IWindowInitializer.h"
#include "AWindow.h"

class CommonWindowInitializer: public IWindowInitializer {
public:
    ~CommonWindowInitializer() override = default;
    void initNativeWindow(AWindow& window, const AString& name, int width, int height, WindowStyle ws,
                          AWindow* parent) override;

    void destroyNativeWindow(AWindow& window) override;
};