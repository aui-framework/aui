//
// Created by alex2 on 02.01.2021.
//

#include "TextColor.h"

void ass::decl::Declaration<ass::TextColor>::applyFor(AView* view) {
    view->getFontStyle().color = mInfo.color;
    view->invalidateFont();
}