/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 01.01.2021.
//

#include "Border.h"
#include <AUI/Render/Render.h>
#include <AUI/Render/RenderHints.h>


void ass::decl::Declaration<ass::Border>::renderFor(AView* view) {
    /*
    if (view->getBorderRadius() < 0.1f) {
        Render::drawRectBorder(ASolidBrush{mInfo.color},
                               {0, 0},
                               view->getSize(),
                               mInfo.width);
    } else {
        */
        Render::drawRectBorder(ASolidBrush{mInfo.color},
                               {0, 0},
                               view->getSize(),
                               view->getBorderRadius(),
                               mInfo.width);
    //}
}

bool ass::decl::Declaration<ass::Border>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::Border>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}