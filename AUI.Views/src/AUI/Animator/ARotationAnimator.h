#pragma once


#include "AAnimator.h"

class API_AUI_VIEWS ARotationAnimator: public AAnimator {
    void doAnimation(AView* view, float theta) override;
};

