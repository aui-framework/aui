//
// Created by alex2 on 03.01.2021.
//

#include "FixedSize.h"


void ass::decl::Declaration<ass::FixedSize>::applyFor(AView* view) {
    view->setFixedSize({mInfo.width, mInfo.height});
}