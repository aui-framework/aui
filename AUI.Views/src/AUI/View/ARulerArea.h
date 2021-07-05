#pragma once

#include <AUI/View/AViewContainer.h>
#include "ARulerView.h"

class API_AUI_VIEWS ARulerArea: public AViewContainer {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;
    glm::ivec2 mMousePos;

    void setWrappedViewPosition(const glm::ivec2& pos);

protected:
    void updatePosition();

public:
    ARulerArea(const _<AView>& wrappedView);
    void setSize(int width, int height) override;
    void onMouseMove(glm::ivec2 pos) override;
    void render() override;
};

