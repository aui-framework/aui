//
// Created by dervisdev on 1/20/2023.
//

#include "ATextOverflow.h"


void ass::prop::Property<ATextOverflow>::applyFor(AView* view) {
    view->setTextOverflow(mInfo);
}
