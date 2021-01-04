//
// Created by alex2 on 31.12.2020.
//

#include "AAssSelector.h"
#include <AUI/View/AView.h>

bool ass::IAssSubSelector::isStateApplicable(AView* view) {
    return true;
}

void ass::IAssSubSelector::setupConnections(AView* view, const _<AAssHelper>& helper) {
}
