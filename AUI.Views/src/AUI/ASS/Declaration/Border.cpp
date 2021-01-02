//
// Created by alex2 on 01.01.2021.
//

#include "Border.h"
#include <AUI/Render/Render.h>
#include <AUI/Render/RenderHints.h>


void ass::decl::Declaration<ass::Border>::renderFor(AView* view) {
    RenderHints::PushColor x;
    if (view->getBorderRadius() > 0) {
        Render::inst().setColor(mInfo.color);
        Render::inst().drawRoundedBorder(0,
                                         0,
                                         view->getWidth(),
                                         view->getHeight(),
                                         view->getBorderRadius(),
                                         mInfo.width);
    } else {
        Render::inst().setFill(Render::FILL_SOLID);
        RenderHints::PushMask mask([&]() {
            Render::inst().drawRect(mInfo.width,
                                    mInfo.width,
                                    view->getWidth() - mInfo.width * 2,
                                    view->getHeight() - mInfo.width * 2);
        });
        RenderHints::PushMask::Layer maskLayer(RenderHints::PushMask::Layer::DECREASE);
        Render::inst().setColor(mInfo.color);
        Render::inst().drawRect(0, 0, view->getWidth(), view->getHeight());
    }
}

bool ass::decl::Declaration<ass::Border>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::Border>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}