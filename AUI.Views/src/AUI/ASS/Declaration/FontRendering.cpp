//
// Created by alex2 on 01.01.2021.
//

#include "FontRendering.h"

void ass::decl::Declaration<FontRendering>::applyFor(AView* view){
    view->getFontStyle().fontRendering = mInfo;
#ifdef __ANDROID__
    if (view->getFontStyle().fontRendering == FontRendering::SUBPIXEL) {
        view->getFontStyle().fontRendering = FontRendering::ANTIALIASING;
    }
#endif
    view->invalidateFont();
}
