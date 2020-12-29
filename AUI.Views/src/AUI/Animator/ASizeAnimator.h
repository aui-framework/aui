//
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AAnimator.h"

class API_AUI_VIEWS ASizeAnimator: public AAnimator {
private:
    glm::ivec2 mBeginSize;
    glm::ivec2 mEndSize;

public:
    ASizeAnimator(const glm::ivec2& beginSize = {0, 0}, const glm::ivec2& endSize = {0, 0});

    void doAnimation(AView* view, float theta) override;
};

