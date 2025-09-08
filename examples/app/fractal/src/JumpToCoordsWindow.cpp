/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

using namespace declarative;

JumpToCoordsWindow::JumpToCoordsWindow(_<FractalView> fractalView, AWindow* parent)
  : AWindow("Jump to coords", 854_dp, 500_dp, parent, WindowStyle::NO_RESIZE) {
    auto re = _new<ATextField>();
    auto im = _new<ATextField>();
    auto scale = _new<ATextField>();

    auto pos = fractalView->getPlotPosition();
    re->setText(AString::number(pos.x));
    im->setText(AString::number(pos.y));
    scale->setText(AString::number(fractalView->getPlotScale()));

    setContents(Vertical {
      _form({
        { "Re="_as, re },
        { "Im="_as, im },
        { "Scale="_as, scale },
      }),
      Horizontal {
        SpacerExpanding {},
        _new<AButton>("Jump").connect(
            &AButton::clicked, this,
            [&, fractalView, re, im, scale]() {
                try {
                    auto dRe = std::stod((*re->text()).toStdString());
                    auto dIm = -std::stod((*re->text()).toStdString());
                    auto dScale = std::stod((*re->text()).toStdString());
                    fractalView->setPlotPositionAndScale(glm::dvec2 { dRe, dIm }, dScale);
                    close();
                } catch (...) {
                    AMessageBox::show(this, "Error", "Please check your values are valid numbers.");
                }
            }) AUI_LET { it->setDefault(); },
        _new<AButton>("Cancel").connect(&AButton::clicked, me::close),
      },
    });

    pack();
}
