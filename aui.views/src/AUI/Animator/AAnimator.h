// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/AVector.h>
#include <glm/gtc/matrix_transform.hpp>

class AView;

namespace AnimationCurves {
    inline float linear(float x) {
        return x;
    }
    class Linear {
    public:
        float operator()(float x) {
            return x;
        }
    };


    /**
     * Use online configurators like https://cubic-bezier.com/
     */
    class CubicBezier {
    private:
        glm::vec2 v1, v2;
    public:
        CubicBezier(const glm::vec2& v1, const glm::vec2& v2) : v1(v1), v2(v2) {}

        float operator()(float t) {
            float oneMinusX = 1 - t;
            float oneMinusX2 = oneMinusX * oneMinusX;
            float oneMinusX3 = oneMinusX2 * oneMinusX;
            float x2 = t * t;
            float x3 = x2 * t;

            float result = 3 * v1.x * oneMinusX2 * t + 3 * v2.x * oneMinusX * x2 + 2 * x3 +
                           3 * v1.y * oneMinusX2 * t + 3 * v2.y * oneMinusX * x2 - t;
            return result;
        }
    };

    class Standard: public CubicBezier {
    public:
        Standard(): CubicBezier({ 0.4f, 0.0f }, { 0.2f, 1.f }) {}
    };
}

class API_AUI_VIEWS AAnimator {
private:
    AView* mView = nullptr;

    float mCurrentTheta = 0;
    float mDuration = 1;

    bool mIsPlaying = true;
    bool mIsRepeating = false;

    std::chrono::milliseconds mLastFrameTime = std::chrono::milliseconds(0);

    std::function<float(float)> mCurve = AnimationCurves::Standard();

protected:
    /**
     * @brief Does the animation (sets matrix, colors, etc...)
     * @param theta animation time value [0;1] where 0 - start of the animation, 1 - end of the animation
     */
    virtual void doAnimation(AView* view, float theta) {};

    virtual void doPostRender(AView* view, float theta) {};

    void translateToCenter();
    void translateToCorner();

public:

    void setCurve(const std::function<float(float)>& curve) {
        mCurve = curve;
    }

    void animate(AView* view);
    void postRender(AView* view);
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

