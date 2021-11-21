//
// Created by alex2 on 08.01.2021.
//

#include <AUI/Render/RenderHints.h>
#include "BorderBottom.h"

void ass::decl::Declaration<BorderBottom>::renderFor(AView* view) {
    RenderHints::PushColor x;
    int w = mInfo.width;
    Render::drawRect(ASolidBrush { mInfo.color },
                     {0, view->getHeight() - w},
                     {view->getWidth(), w });
}

bool ass::decl::Declaration<BorderBottom>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<BorderBottom>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}