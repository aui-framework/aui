//
// Created by alex2772 on 8/20/20.
//

#include <AUI/Platform/AWindow.h>
#include "AAnimator.h"

void AAnimator::animate(AView* view) {
    if (mIsPlaying) {
        AWindow::current()->flagRedraw();
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch());

        if (mLastFrameTime.count() != 0) {
            mCurrentTheta += float((now - mLastFrameTime).count()) / 1000.f / mDuration;
            if (mCurrentTheta > 1.f) {
                if (mIsRepeating) {
                    mCurrentTheta = glm::mod(mCurrentTheta, 1.f);
                } else {
                    mCurrentTheta = 1.f;
                    pause();
                }
            }
        }

        mLastFrameTime = now;
    }
    doAnimation(view, mCurrentTheta);
}

void AAnimator::postRender(AView* view) {
    doPostRender(view, mCurrentTheta);
}

void AAnimator::pause() {
    mLastFrameTime = std::chrono::milliseconds(0);
    mIsPlaying = false;
}

void AAnimator::translateToCenter() {
    Render::inst().setTransform(
            glm::translate(glm::mat4(1.f),
                           glm::vec3(glm::vec2(mView->getSize().x,
                                                mView->getSize().y + mView->getTotalFieldVertical() - 1) / 2.f, 0.f)));
}

void AAnimator::translateToCorner() {
    Render::inst().setTransform(
            glm::translate(glm::mat4(1.f),
                           glm::vec3(-glm::vec2(mView->getSize().x,
                                   mView->getSize().y + mView->getTotalFieldVertical() - 1) / 2.f, 0.f)));
}
