//
// Created by alex2 on 31.12.2020.
//

#include "Selector.h"
#include <AUI/View/AView.h>

bool ass::ISubSelector::isStateApplicable(AView* view) {
    return true;
}

void ass::ISubSelector::setupConnections(AView* view, const _<AAssHelper>& helper) {
}
