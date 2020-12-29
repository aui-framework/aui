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
    AWindow("Jump to coords", 854, 500, parent, WS_NO_RESIZE)
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
        _new<ASpacer>(),
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
        }) let (AButton, {
            setDefault();
        }),
        _new<AButton>("Cancel").connect(&AButton::clicked, me::close)
    }));

    pack();
}
