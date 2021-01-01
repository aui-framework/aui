//
// Created by alex2 on 01.01.2021.
//

#include "BorderRadius.h"

void ass::decl::Declaration<ass::BorderRadius>::applyFor(AView* view) {
    view->setBorderRadius(mInfo.radius);
}