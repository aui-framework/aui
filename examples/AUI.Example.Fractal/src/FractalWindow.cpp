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


FractalWindow::FractalWindow():
    AWindow("Mandelbrot set")
{
    setLayout(_new<AHorizontalLayout>());

    auto centerPosDisplay = _new<ALabel>("-");
    centerPosDisplay->setCss("background: #0008; padding: 4em; color: #fff; font-size: 11pt");

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
        }) let (AViewContainer, {
            setExpanding({2, 2});
        })
    }) let (AViewContainer, {
        setExpanding({2, 2});
    }));


    addView(
        _container<AVerticalLayout>({
            _new<AButton>("Identity").connect(&AButton::clicked, slot(fractal)::reset),
            _new<AButton>("Jump to coords...").connect(&AButton::clicked, this, [&, fractal]() {
                _new<JumpToCoordsWindow>(fractal, this)->show();
            }),
            _new<ALabel>("Iterations:"),
            _new<ANumberPicker>().connect(&ANumberPicker::valueChanged, this, [fractal](int v) {
                fractal->setIterations(v);
            }) let (ANumberPicker, {
                setMax(1000);
                setValue(350);
            }),
            _new<ALabel>("Gradient:"),
            _new<AImageView>(fractal->getTexture()) let (AImageView, {
                setCss("height: 10em; margin: 4em;");
            }),
    }));
    fractal->focus();
}
