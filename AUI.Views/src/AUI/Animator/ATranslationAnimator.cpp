//
// Created by alex2 on 27.08.2020.
//

#include "ATranslationAnimator.h"
#include <AUI/Render/Render.h>

ATranslationAnimator::ATranslationAnimator(const glm::vec2& from, const glm::vec2& to) : mFrom(from), mTo(to) {}

void ATranslationAnimator::doAnimation(AView* view, float theta) {
    Render::inst().setTransform(glm::translate(glm::mat4(1.f),
                                               glm::vec3(glm::mix(mFrom, mTo, theta), 0.f)));
}
