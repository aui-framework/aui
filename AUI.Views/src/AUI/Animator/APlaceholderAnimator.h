//
// Created by alex2 on 28.11.2020.
//

#pragma once


#include "AAnimator.h"

class API_AUI_VIEWS APlaceholderAnimator: public AAnimator {
public:
    APlaceholderAnimator();


    class ICustomWidth {
    public:

        virtual ~ICustomWidth() = default;
        virtual float getCustomWidthForPlaceholderAnimator() = 0;
    };

protected:
    void doPostRender(AView* view, float theta) override;
};


