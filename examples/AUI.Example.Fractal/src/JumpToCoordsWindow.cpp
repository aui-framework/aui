// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
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
                auto dRe = std::stod(re->text().toStdString());
                auto dIm = -std::stod(im->text().toStdString());
                auto dScale = std::stod(scale->text().toStdString());
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
