// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
#include <AUI/ASS/ASS.h>

using namespace ass;

FractalWindow::FractalWindow():
    AWindow("Mandelbrot set")
{
    setLayout(_new<AHorizontalLayout>());

    auto centerPosDisplay = _new<ALabel>("-");
    {
        centerPosDisplay->setCustomStyle({
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
            _new<ASpacerExpanding>(),
            _container<AHorizontalLayout>({
                _new<ASpacerExpanding>(),
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
    }));
    fractal->focus();
}
