//
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AAnimator.h"

class API_AUI_VIEWS ATranslationAnimator: public AAnimator {
private:
    glm::vec2 mFrom;
    glm::vec2 mTo;

public:
    ATranslationAnimator(const glm::vec2& from, const glm::vec2& to = {0.f, 0.f});

    void doAnimation(AView* view, float theta) override;
};

