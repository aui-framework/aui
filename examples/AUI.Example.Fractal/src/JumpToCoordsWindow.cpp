/*
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
// Created by alex2772 on 12/10/20.
//

#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/Layout/AAdvancedGridLayout.h>
#include "JumpToCoordsWindow.h"
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/ATextField.h>
#include <AUI/View/AButton.h>
#include <AUI/Platform/AMessageBox.h>

JumpToCoordsWindow::JumpToCoordsWindow(_<FractalView> fractalView, AWindow* parent):
    AWindow("Jump to coords", 854_dp, 500_dp, parent, WindowStyle::NO_RESIZE)
{
    setLayout(_new<AVerticalLayout>());

    auto re = _new<ATextField>();
    auto im = _new<ATextField>();
    auto scale = _new<ATextField>();


    auto pos = fractalView->getPlotPosition();
    re->setText(AString::number(pos.x));
    im->setText(AString::number(pos.y));
    scale->setText(AString::number(fractalView->getPlotScale()));

    addView(_form({
        {"Re="_as, re},
        {"Im="_as, im},
        {"Scale="_as, scale},
    }));

    addView(_container<AHorizontalLayout>({
        _new<ASpacerExpanding>(),
        _new<AButton>("Jump").connect(&AButton::clicked, this, [&, fractalView, re, im, scale]() {
            try {
                auto dRe = std::stod(re->getText().toStdString());
                auto dIm = -std::stod(im->getText().toStdString());
                auto dScale = std::stod(scale->getText().toStdString());
                fractalView->setPlotPositionAndScale(glm::dvec2{dRe, dIm}, dScale);
                close();
            } catch (...) {
                AMessageBox::show(this, "Error", "Please check your values are valid numbers.");
            }
        }) let {
            it->setDefault();
        },
        _new<AButton>("Cancel").connect(&AButton::clicked, me::close)
    }));

    pack();
}
