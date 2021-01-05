#pragma once

#include <AUI/Common/AObject.h>
#include <glm/gtc/matrix_transform.hpp>

class AView;

class API_AUI_VIEWS AAnimator {
private:
    AView* mView = nullptr;

    float mCurrentTheta = 0;
    float mDuration = 1;

    bool mIsPlaying = true;
    bool mIsRepeating = false;

    std::chrono::milliseconds mLastFrameTime = std::chrono::milliseconds(0);

protected:
    /**
     * \brief Does the animation (sets matrix, colors, etc...)
     * \param theta animation time value [0;1] where 0 - start of the animation, 1 - end of the animation
     */
    virtual void doAnimation(AView* view, float theta) {};

    virtual void doPostRender(AView* view, float theta) {};

    void translateToCenter();
    void translateToCorner();

public:

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
};

