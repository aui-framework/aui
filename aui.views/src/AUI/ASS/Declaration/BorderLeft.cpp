//
// Created by alex2 on 08.01.2021.
//

#include <AUI/Render/RenderHints.h>
#include "BorderLeft.h"

void ass::decl::Declaration<ass::BorderLeft>::renderFor(AView* view) {
    RenderHints::PushColor x;
    int w = mInfo.width;

    Render::rect(ASolidBrush{mInfo.color},
                 {0, 0},
                 {w, view->getHeight()});

}

bool ass::decl::Declaration<ass::BorderLeft>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::decl::DeclarationSlot ass::decl::Declaration<ass::BorderLeft>::getDeclarationSlot() const {
    return ass::decl::DeclarationSlot::BORDER;
}