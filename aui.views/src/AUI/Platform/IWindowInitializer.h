#pragma once


#include "AWindowNativePtr.h"
#include "IWindowInitializer.h"
#include <AUI/Common/AString.h>
#include <AUI/Enum/WindowStyle.h>

class AWindow;

class IWindowInitializer {
public:
    virtual ~IWindowInitializer() = default;
    virtual void initNativeWindow(AWindow& window,
                                  const AString& name,
                                  int width,
                                  int height,
                                  WindowStyle ws,
                                  AWindow* parent) = 0;
    virtual void destroyNativeWindow(AWindow& window) = 0;

    virtual void beginPaint(AWindow& window) = 0;
    virtual void endPaint(AWindow& window) = 0;
    virtual void beginResize(AWindow& window) = 0;
};