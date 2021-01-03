//
// Created by alex2772 on 1/3/21.
//

#include "Overflow.h"


void ass::decl::Declaration<AView::Overflow>::applyFor(AView* view) {
    view->setOverflow(mInfo);
}