#pragma once


#include "IBackgroundEffect.h"

class API_AUI_VIEWS EffectShimmer: public IBackgroundEffect {
public:
    EffectShimmer(AColor color = AColor::WHITE.transparentize(0.5f)): mColor(color) {}

    void draw(AView* view) override;

private:
    AColor mColor;
};


