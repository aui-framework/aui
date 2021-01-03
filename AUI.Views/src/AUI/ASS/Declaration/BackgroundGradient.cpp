//
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "BackgroundGradient.h"
#include "IDeclaration.h"



void ass::decl::Declaration<ass::BackgroundGradient>::renderFor(AView* view) {
    RenderHints::PushColor x;

    Render::inst().setFill(Render::FILL_GRADIENT);
    if (mInfo.direction == LayoutDirection::VERTICAL) {
        Render::inst().setGradientColors(mInfo.topLeftColor,
                                         mInfo.topLeftColor,
                                         mInfo.bottomRightColor,
                                         mInfo.bottomRightColor);
    } else {
        Render::inst().setGradientColors(mInfo.topLeftColor,
                                         mInfo.bottomRightColor,
                                         mInfo.topLeftColor,
                                         mInfo.bottomRightColor);
    }
    if (view->getBorderRadius() > 0) {
        Render::inst().drawRoundedRectAntialiased(0, 0, view->getWidth(), view->getHeight(), view->getBorderRadius());
    } else  {
        Render::inst().drawRect(0, 0, view->getWidth(), view->getHeight());
    }
    IDeclarationBase::renderFor(view);
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BackgroundGradient>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BACKGROUND_SOLID;
}


