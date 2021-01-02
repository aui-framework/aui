//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "SolidBackground.h"
#include "IDeclaration.h"


void ass::decl::Declaration<ass::SolidBackground>::renderFor(AView* view) {
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

ass::decl::DeclarationSlot ass::decl::Declaration<ass::SolidBackground>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND;
}


