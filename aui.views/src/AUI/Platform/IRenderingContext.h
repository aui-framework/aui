#pragma once


#include "AWindowNativePtr.h"
#include "IRenderingContext.h"
#include <AUI/Common/AString.h>
#include <AUI/Enum/WindowStyle.h>

class AWindow;

class IRenderingContext {
public:
    struct Init {
        AWindow& window;
        AString name;
        int width;
        int height;
        WindowStyle ws;
        AWindow* parent;
    };
    virtual void init(const Init& init);
    virtual ~IRenderingContext() = default;
    virtual void destroyNativeWindow(AWindow& window) = 0;

    virtual void beginPaint(AWindow& window) = 0;
    virtual void endPaint(AWindow& window) = 0;
    virtual void beginResize(AWindow& window) = 0;
};