// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
    doAnimation(view, mCurve(mCurrentTheta));
}

void AAnimator::postRender(AView* view) {
    doPostRender(view, mCurrentTheta);
}

void AAnimator::pause() {
    mLastFrameTime = std::chrono::milliseconds(0);
    mIsPlaying = false;
}

void AAnimator::translateToCenter() {
    translateToCenter(mView);
}

void AAnimator::translateToCorner() {
    translateToCorner(mView);
}

void AAnimator::translateToCenter(AView* view) {
    Render::setTransform(
            glm::translate(glm::mat4(1.f),
                           glm::vec3(glm::vec2(view->getSize().x,
                                               view->getSize().y + view->getTotalFieldVertical() - 1) / 2.f, 0.f)));
}

void AAnimator::translateToCorner(AView* view) {
    Render::setTransform(
            glm::translate(glm::mat4(1.f),
                           glm::vec3(-glm::vec2(view->getSize().x,
                                                view->getSize().y + view->getTotalFieldVertical() - 1) / 2.f, 0.f)));
}

_<AAnimator> AAnimator::combine(const AVector<_<AAnimator>>& animators) {
    class ACombiningAnimator: public AAnimator {
    private:
        AVector<_<AAnimator>> mAnimator;

    public:
        ACombiningAnimator(const AVector<_<AAnimator>>& animator) : mAnimator(animator) {}

    protected:
        void doAnimation(AView* view, float theta) override {
            for (auto& animator : mAnimator) {
                animator->animate(view);
            }
        }

        void doPostRender(AView* view, float theta) override {
            for (auto& animator : mAnimator) {
                animator->doPostRender(view, theta);
            }
        }
    };
    return _new<ACombiningAnimator>(animators);
}
