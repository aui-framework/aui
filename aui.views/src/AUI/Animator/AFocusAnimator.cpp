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
// Created by alex2 on 18.09.2020.
//

#include "AFocusAnimator.h"
#include <AUI/Render/Render.h>
#include <AUI/View/AView.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Platform/AWindow.h>

AFocusAnimator::AFocusAnimator() {
}

void AFocusAnimator::doAnimation(AView* view, float theta) {
    const float SIZE = 4.f;
    if (theta < 0.99999f) {
        RenderHints::PushColor c;
        Render::setFill(Render::FILL_SOLID);
        float t = glm::pow(1.f - theta, 4.f);
        Render::setColor({0, 0, 0, t});
        Render::drawRectBorder(-t * SIZE, -t * SIZE, t * SIZE * 2 + view->getWidth(),
                                          t * SIZE * 2 + view->getHeight(), 2.f);
    }
}
