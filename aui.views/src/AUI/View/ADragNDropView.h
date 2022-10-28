#pragma once


#include "AViewContainer.h"

class API_AUI_VIEWS ADragNDropView: public AViewContainer {
public:
    ADragNDropView();

    void onMouseMove(glm::ivec2 pos) override;

private:
    bool mDnDInProgress = false;
};


