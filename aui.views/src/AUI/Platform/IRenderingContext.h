#pragma once


#include "AWindowNativePtr.h"
#include "IRenderingContext.h"
#include <AUI/Common/AString.h>
#include <AUI/Enum/WindowStyle.h>
#include <AUI/Image/AImage.h>

class AWindow;

class API_AUI_VIEWS IRenderingContext {
public:
    struct API_AUI_VIEWS Init {
        AWindow& window;
        AString name;
        int width;
        int height;
        WindowStyle ws;
        AWindow* parent;

        void setRenderingContext(_unique<IRenderingContext>&& context) const;
    };
    virtual void init(const Init& init);
    virtual ~IRenderingContext() = default;
    virtual void destroyNativeWindow(AWindow& window) = 0;

    virtual AImage makeScreenshot() = 0;

    virtual void beginPaint(AWindow& window) = 0;
    virtual void endPaint(AWindow& window) = 0;
    virtual void beginResize(AWindow& window) = 0;
    virtual void endResize(AWindow& window) = 0;
};