//
// Created by dervisdev on 1/26/2023.
//

#include "ScrollbarAppearance.h"
#include <AUI/View/AScrollArea.h>

void ass::prop::Property<ass::ScrollbarAppearance>::applyFor(AView* view) {
    if (auto scrollArea = _cast<AScrollArea>(view->sharedPtr())) {
        scrollArea->setScrollbarAppearance(mInfo);
    }
}
