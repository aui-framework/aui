//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "BackgroundSolid.h"
#include "IDeclaration.h"


void ass::decl::Declaration<ass::BackgroundSolid>::renderFor(AView* view) {
    RenderHints::PushColor x;

    Render::inst().setColor(mInfo.color);
    Render::inst().setFill(Render::FILL_SOLID);
    if (view->getBorderRadius() > 0) {
        Render::inst().drawRoundedRectAntialiased(0, 0, view->getWidth(), view->getHeight(), view->getBorderRadius());
    } else  {
        Render::inst().drawRect(0, 0, view->getWidth(), view->getHeight());
    }
    IDeclarationBase::renderFor(view);
}

bool ass::decl::Declaration<ass::BackgroundSolid>::isNone() {
    return mInfo.color.isFullyTransparent();
}
ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundSolid>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_SOLID;
}


