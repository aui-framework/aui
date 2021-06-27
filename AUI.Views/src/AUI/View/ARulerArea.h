#pragma once

#include <AUI/View/AViewContainer.h>
#include "ARulerView.h"

class API_AUI_VIEWS ARulerArea: public AViewContainer {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;

public:
    ARulerArea(const _<AView>& wrappedView);
};

