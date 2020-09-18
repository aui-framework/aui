//
// Created by alex2 on 18.09.2020.
//

#pragma once

#include "AAnimator.h"

class API_AUI_VIEWS AFocusAnimator: public AAnimator {
public:
    AFocusAnimator();
    void doAnimation(AView* view, float theta) override;
};

