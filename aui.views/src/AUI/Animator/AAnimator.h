/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/AVector.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Curves/linear.h"
#include "Curves/bezier.h"
#include "Curves/viscous_fluid.h"

class AView;
class IRenderer;

using AAnimationCurve = std::function<float(float)>;

class API_AUI_VIEWS AAnimator {
private:
    AView* mView = nullptr;

    float mCurrentTheta = 0;
    float mDuration = 1;

    bool mIsPlaying = true;
    bool mIsRepeating = false;

    std::chrono::milliseconds mLastFrameTime = std::chrono::milliseconds(0);

    AAnimationCurve mCurve = aui::animation_curves::Standard();

protected:
    /**
     * @brief Does the animation (sets matrix, colors, etc...)
     * @param view target view to perform animation for.
     * @param theta animation time value [0;1] where 0 - start of the animation, 1 - end of the animation.
     * @param render renderer.
     */
    virtual void doAnimation(AView* view, float theta, IRenderer& render) {};

    virtual void doPostRender(AView* view, float theta, IRenderer& render) {};

    void translateToCenter(IRenderer& render);
    void translateToCorner(IRenderer& render);

    static void translateToCenter(AView* view, IRenderer& render);
    static void translateToCorner(AView* view, IRenderer& render);

public:

    void setCurve(const std::function<float(float)>& curve) {
        mCurve = curve;
    }

    void animate(AView* view, IRenderer& render);
    void postRender(AView* view, IRenderer& render);
    void pause();

    void setDuration(float period) {
        mDuration = period;
    }

    void setRepeating(bool repeating) {
        mIsRepeating = repeating;
    }

    [[nodiscard]] const AView* getView() const {
        return mView;
    }

    void setView(AView* view) {
        mView = view;
    }

    static _<AAnimator> combine(const AVector<_<AAnimator>>& animators);
};