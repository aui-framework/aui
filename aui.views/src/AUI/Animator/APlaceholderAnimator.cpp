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
// Created by alex2 on 28.11.2020.
//

#include "APlaceholderAnimator.h"
#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Render/Render.h>

APlaceholderAnimator::APlaceholderAnimator() {

}

void APlaceholderAnimator::doPostRender(AView* view, float theta) {
    float width;
    if (auto cs = dynamic_cast<ICustomWidth*>(view)) {
        width = cs->getCustomWidthForPlaceholderAnimator();
    } else {
        width = view->getWidth();
    }
    width = glm::max(width, 80.f);
    setDuration(width / float(200_dp));

    Render::setFill(Render::FILL_GRADIENT);
    auto BRIGHT_COLOR = 0x40ffffff_argb;
    Render::setGradientColors(0x00ffffff_argb, BRIGHT_COLOR,
                                     0x00ffffff_argb, BRIGHT_COLOR);

    const float WIDTH = 200;
    float posX = theta * (view->getWidth() + WIDTH * 2.f) - WIDTH;

    Render::drawRect(posX, 0, WIDTH / 2, view->getHeight());

    Render::setGradientColors(BRIGHT_COLOR, 0x00ffffff_argb,
                                     BRIGHT_COLOR, 0x00ffffff_argb);
    Render::drawRect(posX + WIDTH / 2, 0, WIDTH / 2, view->getHeight());
}
