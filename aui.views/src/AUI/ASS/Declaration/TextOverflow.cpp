//
// Created by dervisdev on 1/17/2023.
//

#include "TextOverflow.h"


void ass::decl::Declaration<ATextOverflow>::applyFor(AView* view) {
    view->setTextOverflow(mInfo);
}