//
// Created by dervisdev on 1/26/2023.
//

#include "ScrollbarAppearance.h"
#include <AUI/View/AScrollArea.h>

void ass::decl::Declaration<ScrollbarAppearance>::applyFor(AView* view) {
    if (auto scrollArea = _cast<AViewContainer>(view->sharedPtr())) {
        scrollArea->setScrollbarAppearance(mInfo);
    }
}
