//
// Created by Alex2772 on 12/12/2021.
//

#include <AUI/Render/Render.h>
#include "IRenderingContext.h"
#include <AUI/Platform/AWindow.h>

void IRenderingContext::init(const IRenderingContext::Init& init) {
    init.window.updateDpi();
}

void IRenderingContext::Init::setRenderingContext(_unique<IRenderingContext>&& context) const {
    window.mRenderingContext = std::forward<_unique<IRenderingContext>>(context);
}
