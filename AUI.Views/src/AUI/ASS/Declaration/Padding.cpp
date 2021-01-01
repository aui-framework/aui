//
// Created by alex2 on 01.01.2021.
//

#include "Padding.h"

void ass::decl::Declaration<ass::Padding>::applyFor(AView* view) {
    view->setPadding(mInfo.padding);
}