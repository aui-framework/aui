//
// Created by alex2772 on 8/20/20.
//

#include "ARotationAnimator.h"
#include <AUI/Render/Render.h>

void ARotationAnimator::doAnimation(AView* view, float theta) {

    translateToCenter();
    Render::inst().setTransform(
            glm::rotate(glm::mat4(1.f), theta * 2 * glm::pi<float>(), glm::vec3{0.f, 0.f, 1.f}));
    translateToCorner();

}
