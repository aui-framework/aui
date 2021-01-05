/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
