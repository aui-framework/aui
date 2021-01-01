//
// Created by alex2 on 01.01.2021.
//

#include "TextAlign.h"


void ass::decl::Declaration<TextAlign>::applyFor(AView* view) {
    view->getFontStyle().align = mInfo;
}