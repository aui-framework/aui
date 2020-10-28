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
     * \brief Выполнить непосредственно анимацию (выставить матрицы, цвета и так далее)
     * \param theta число [0;1], где 0 - начало анимации, 1 - конец анимации
     */
    virtual void doAnimation(AView* view, float theta) = 0;

    void translateToCenter();
    void translateToCorner();

public:

    void animate(AView* view);
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

