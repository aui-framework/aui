//
// Created by alex2 on 01.01.2021.
//

#include "Margin.h"

void ass::decl::Declaration<ass::Margin>::applyFor(AView* view) {
    view->setMargin(mInfo.margin);
}