//
// Created by alex2 on 28.11.2020.
//

#include "APlaceholderAnimator.h"
#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Render/Render.h>

APlaceholderAnimator::APlaceholderAnimator() {

}

void APlaceholderAnimator::doPostRender(AView* view, float theta) {
    float width;
    if (auto cs = dynamic_cast<ICustomWidth*>(view)) {
        width = cs->getCustomWidthForPlaceholderAnimator();
    } else {
        width = view->getWidth();
    }
    width = glm::max(width, 80.f);
    setDuration(width / float(200_dp));

    Render::inst().setFill(Render::FILL_GRADIENT);
    auto BRIGHT_COLOR = 0x40ffffff_argb;
    Render::inst().setGradientColors(0x00ffffff_argb, BRIGHT_COLOR,
                                     0x00ffffff_argb, BRIGHT_COLOR);

    const float WIDTH = 200;
    float posX = theta * (view->getWidth() + WIDTH * 2.f) - WIDTH;

    Render::inst().drawRect(posX, 0, WIDTH / 2, view->getHeight());

    Render::inst().setGradientColors(BRIGHT_COLOR, 0x00ffffff_argb,
                                     BRIGHT_COLOR, 0x00ffffff_argb);
    Render::inst().drawRect(posX + WIDTH / 2, 0, WIDTH / 2, view->getHeight());
}
