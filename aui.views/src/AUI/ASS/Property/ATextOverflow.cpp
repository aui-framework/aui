//
// Created by dervisdev on 1/20/2023.
//

#include <AUI/View/AAbstractLabel.h>
#include "ATextOverflow.h"


void ass::prop::Property<ATextOverflow>::applyFor(AView* view) {
    AUI_NULLSAFE(dynamic_cast<AAbstractLabel*>(view))->setTextOverflow(mInfo);
}
