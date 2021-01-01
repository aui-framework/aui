//
// Created by alex2 on 01.01.2021.
//

#include "MinSize.h"

void ass::decl::Declaration<ass::MinSize>::applyFor(AView* view) {
    view->setMinSize({mInfo.width.getValuePx(), mInfo.height.getValuePx()});
}