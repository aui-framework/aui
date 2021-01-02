//
// Created by alex2 on 01.01.2021.
//

#include "BoxShadow.h"
#include <AUI/Render/Render.h>


void ass::decl::Declaration<ass::BoxShadow>::renderFor(AView* view) {
    Render::inst().drawBoxShadow(mInfo.offsetX - mInfo.spreadRadius,
                                 mInfo.offsetY - mInfo.spreadRadius,
                                 view->getWidth() + mInfo.spreadRadius * 2,
                                 view->getHeight() + mInfo.spreadRadius * 2,
                                 mInfo.blurRadius,
                                 mInfo.color);
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BoxShadow>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::SHADOW;
}

bool ass::decl::Declaration<ass::BoxShadow>::isNone() {
    return mInfo.color.isFullyTransparent();
}