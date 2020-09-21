//
// Created by alex2 on 22.09.2020.
//

#include "ASizeAnimator.h"
#include <AUI/View/AView.h>

ASizeAnimator::ASizeAnimator(const glm::ivec2& beginSize) : mBeginSize(beginSize), mEndSize(0) {

}

void ASizeAnimator::doAnimation(AView* view, float theta) {
    if (mEndSize.x == 0 && mEndSize.y == 0) {
        mEndSize = view->getSize();
    }
    auto currentSize = glm::mix(mBeginSize, mEndSize, theta);
    view->setSize(currentSize.x, currentSize.y);
}

