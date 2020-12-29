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
        Render::inst().setFill(Render::FILL_SOLID);
        float t = glm::pow(1.f - theta, 4.f);
        Render::inst().setColor({0, 0, 0, t});
        Render::inst().drawRectBorder(-t * SIZE, -t * SIZE, t * SIZE * 2 + view->getWidth(),
                                          t * SIZE * 2 + view->getHeight(), 2.f);
    }
}
