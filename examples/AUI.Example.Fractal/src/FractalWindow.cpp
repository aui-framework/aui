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
// Created by alex2772 on 12/9/20.
//

#include <AUI/View/ANumberPicker.h>
#include <AUI/View/AButton.h>
#include <AUI/Traits/strings.h>
#include "FractalWindow.h"
#include "FractalView.h"
#include "JumpToCoordsWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AImageView.h>
#include <AUI/ASS/ASS.h>

using namespace ass;

FractalWindow::FractalWindow():
    AWindow("Mandelbrot set")
{
    setLayout(_new<AHorizontalLayout>());

    auto centerPosDisplay = _new<ALabel>("-");
    {
        centerPosDisplay->setCustomAss({
            BackgroundSolid { 0x80000000_argb },
            Padding { 4_dp },
            TextColor { 0xffffff_rgb },
            FontSize { 11_pt },
        });
    }

    auto fractal = _new<FractalView>();
    connect(fractal->centerPosChanged, this, [centerPosDisplay](const glm::dvec2& newPos, double scale) {
        char buf[1024];
        sprintf(buf, "Center position: %.20lf %.20lf, scale: %.4e", newPos.x, -newPos.y, scale);
        centerPosDisplay->setText(buf);
    });

    addView(_container<AStackedLayout>({
        fractal,
        _container<AVerticalLayout>({
            _new<ASpacer>(),
            _container<AHorizontalLayout>({
                _new<ASpacer>(),
                centerPosDisplay
            })
        }) let {
            it->setExpanding();
        }
    }) let {
        it->setExpanding();
    });


    addView(
        _container<AVerticalLayout>({
            _new<AButton>("Identity").connect(&AButton::clicked, slot(fractal)::reset),
            _new<AButton>("Jump to coords...").connect(&AButton::clicked, this, [&, fractal]() {
                _new<JumpToCoordsWindow>(fractal, this)->show();
            }),
            _new<ALabel>("Iterations:"),
            _new<ANumberPicker>().connect(&ANumberPicker::valueChanged, this, [fractal](int v) {
                fractal->setIterations(v);
            }) let {
                it->setMax(1000);
                it->setValue(350);
            },
            _new<ALabel>("Gradient:"),
            _new<AImageView>(fractal->getTexture()) let {
                it->setCustomAss({
                    FixedSize { {}, 10_dp },
                    Margin { 4_dp }
                });
            },
    }));
    fractal->focus();
}
