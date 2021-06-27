#pragma once

#include <AUI/View/AViewContainer.h>
#include "ARulerView.h"

class API_AUI_VIEWS ARulerArea: public AViewContainer {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;


    void setWrappedViewPosition(const glm::ivec2& pos);

public:
    ARulerArea(const _<AView>& wrappedView);

    void setSize(int width, int height) override;
};

