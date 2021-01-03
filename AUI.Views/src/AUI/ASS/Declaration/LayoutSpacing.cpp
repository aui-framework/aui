//
// Created by alex2772 on 1/3/21.
//

#include "LayoutSpacing.h"
#include <AUI/View/AViewContainer.h>


void ass::decl::Declaration<ass::LayoutSpacing>::applyFor(AView* view) {
    auto container = dynamic_cast<AViewContainer*>(view);
    assert(container);
    if (auto l = container->getLayout()) {
        l->setSpacing(mInfo.spacing);
    }
}