//
// Created by alex2 on 01.01.2021.
//

#include "FontSize.h"


void ass::decl::Declaration<ass::FontSize>::applyFor(AView* view) {
    view->getFontStyle().size = mInfo.size.getValuePx();
}