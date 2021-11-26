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
// Created by alex2 on 23.10.2020.
//

#include "ADrawableView.h"
#include <AUI/Render/Render.h>
#include <AUI/ASS/ASS.h>

ADrawableView::ADrawableView(const _<IDrawable>& drawable) : mDrawable(drawable) {

}

void ADrawableView::render() {
    AView::render();
    Render::setColor(getAssHelper()->state.backgroundUrl.overlayColor.or_default(0xffffff_rgb));
    if (mDrawable) {
        IDrawable::Params p;
        p.size = getSize();
        mDrawable->draw(p);
    }
}

/*
void
ADrawableView::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
    processor(css::T_AUI_BACKGROUND_OVERLAY, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            mColorOverlay = AColor(p->getArgs()[0]);
        }
    });
}
*/