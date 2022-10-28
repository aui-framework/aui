#pragma once

#include <AUI/View/AViewContainer.h>
#include "ARulerView.h"

/**
 * @brief Photoshop-like ruler area for measuring display units.
 * @ingroup useful_views
 * @details
 * Consists of vertical and horizontal @ref ARulerView "rulers".
 */
class API_AUI_VIEWS ARulerArea: public AViewContainer {
private:
    _<AView> mWrappedView;
    _<ARulerView> mHorizontalRuler;
    _<ARulerView> mVerticalRuler;
    glm::ivec2 mMousePos;

    void setWrappedViewPosition(const glm::ivec2& pos);

    glm::ivec2 getTargetPosition() const;

protected:
    void updatePosition();

public:
    ARulerArea(const _<AView>& wrappedView);
    void setSize(glm::ivec2 size) override;
    void onMouseMove(glm::ivec2 pos) override;
    void render() override;
};

