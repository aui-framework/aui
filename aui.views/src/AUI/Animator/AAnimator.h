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

/**
 * @brief Base class for view animations.
 * @ingroup views
 * @details
 * An animator changes how a view is painted over time: it moves, rotates or resizes it. Create an animator, configure
 * and assign it to a view with `AView::setAnimator`. The animation starts immediately, the view is repainted
 * continuously until the animation ends.
 *
 * ```cpp
 * auto animator = _new<ATranslationAnimator>(glm::vec2 { -100, 0 });   // from (-100, 0) to the actual position
 * animator->setDuration(0.3f);                                         // seconds
 * view->setAnimator(animator);
 * ```
 *
 * Ready-to-use animators:
 *
 * - `ATranslationAnimator` moves the view from one offset to another;
 * - `ARotationAnimator` rotates the view between two angles ([AAngleRadians], i.e., `90_deg`);
 * - `ASizeAnimator` changes the size of the view;
 * - `AFocusAnimator` is used by AUI itself for the focus highlight.
 *
 * `setRepeating(true)` makes an animation loop endlessly, which fits a progress indicator. `setCurve` replaces the
 * default easing function with a custom `float(float)`, i.e., `aui::animation_curves::Linear` or
 * `aui::animation_curves::CubicBezier`.
 *
 * Several animators can be merged with `AAnimator::combine({ a, b })`.
 *
 * To implement your own animator, derive from AAnimator and override `doAnimation`, which receives the animation
 * progress `theta` (from 0 to 1) and the renderer to apply a transform to.
 */
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
     * @param theta animation time value from 0 to 1 where 0 - start of the animation, 1 - end of the animation.
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