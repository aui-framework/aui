/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2772 on 8/20/20.
//

#include <AUI/Platform/AWindow.h>
#include "AAnimator.h"

void AAnimator::animate(AView* view, IRenderer& render) {
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
    doAnimation(view, mCurve(mCurrentTheta), render);
}

void AAnimator::postRender(AView* view, IRenderer& render) {
    doPostRender(view, mCurrentTheta, render);
}

void AAnimator::pause() {
    mLastFrameTime = std::chrono::milliseconds(0);
    mIsPlaying = false;
}

void AAnimator::translateToCenter(IRenderer& render) {
    translateToCenter(mView, render);
}

void AAnimator::translateToCorner(IRenderer& render) {
    translateToCorner(mView, render);
}

void AAnimator::translateToCenter(AView* view, IRenderer& render) {
    render.setTransform(
            glm::translate(glm::mat4(1.f),
                           glm::vec3(glm::vec2(view->getSize().x,
                                               view->getSize().y + view->getTotalFieldVertical() - 1) / 2.f, 0.f)));
}

void AAnimator::translateToCorner(AView* view, IRenderer& render) {
    render.setTransform(
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
        void doAnimation(AView* view, float theta, IRenderer& render) override {
            for (auto& animator : mAnimator) {
                animator->animate(view, render);
            }
        }

        void doPostRender(AView* view, float theta, IRenderer& render) override {
            for (auto& animator : mAnimator) {
                animator->doPostRender(view, theta, render);
            }
        }
    };
    return _new<ACombiningAnimator>(animators);
}
