//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "SolidBackground.h"


void ass::decl::Declaration<ass::SolidBackground>::applyFor(AView* view){
    RenderHints::PushColor x;

    Render::inst().setColor(mInfo.color);
    if (view->getBorderRadius() > 0) {
        Render::inst().drawRoundedRectAntialiased(0, 0, view->getWidth(), view->getHeight(), view->getBorderRadius());
    } else  {
        Render::inst().setFill(Render::FILL_SOLID);
        Render::inst().drawRect(0, 0, view->getWidth(), view->getHeight());
    }
}
