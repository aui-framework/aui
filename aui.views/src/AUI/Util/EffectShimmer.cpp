//
// Created by Alex2772 on 8/31/2022.
//

#include "EffectShimmer.h"
#include <AUI/Render/Render.h>

void EffectShimmer::draw(AView* view) {
    using namespace std::chrono;

    const float t = (duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() % 1'000) / 1'000.f;

    const auto transparent = AColor(glm::vec4(glm::vec3(mColor), 0.f));

    Render::rect(ALinearGradientBrush {
        transparent,
        mColor,
        transparent,
        mColor
    }, {(t * 4 - 2) * view->getSize().x, 0}, view->getSize());


    Render::rect(ALinearGradientBrush {
        mColor,
        transparent,
        mColor,
        transparent,
    }, {(t * 4 - 1) * view->getSize().x, 0}, view->getSize());

    view->redraw();
}
