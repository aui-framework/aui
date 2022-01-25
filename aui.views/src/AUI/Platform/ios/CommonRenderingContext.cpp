//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Util/AError.h>
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Util/kAUI.h"


void CommonRenderingContext::init(const Init& init) {
    IRenderingContext::init(init);
}

void CommonRenderingContext::destroyNativeWindow(ABaseWindow& window) {

}

void CommonRenderingContext::beginPaint(ABaseWindow& window) {
}

void CommonRenderingContext::endPaint(ABaseWindow& window) {
}
