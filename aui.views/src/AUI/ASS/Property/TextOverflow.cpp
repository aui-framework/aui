//
// Created by dervisdev on 1/20/2023.
//

#include "TextOverflow.h"


void ass::prop::Property<TextOverflow>::applyFor(AView* view) {
    view->setTextOverflow(mInfo);
}
