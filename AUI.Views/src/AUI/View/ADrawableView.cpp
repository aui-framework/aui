//
// Created by alex2 on 23.10.2020.
//

#include "ADrawableView.h"
#include <AUI/Render/Render.h>

ADrawableView::ADrawableView(const _<IDrawable>& drawable) : mDrawable(drawable), mColorOverlay(1.f) {
    AVIEW_CSS;
}

void ADrawableView::render() {
    AView::render();
    Render::inst().setColor(mColorOverlay);
    if (mDrawable)
        mDrawable->draw(getSize());
}

void
ADrawableView::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor) {
    processor(css::T_AUI_BACKGROUND_OVERLAY, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            mColorOverlay = AColor(p->getArgs()[0]);
        }
    });
}
