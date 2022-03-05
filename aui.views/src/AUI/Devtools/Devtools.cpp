//
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Reflect/AReflect.h>
#include <AUI/View/AViewContainer.h>
#include "Devtools.h"

AString Devtools::prettyViewName(AView* view) {
    AString name;
    if (auto c = dynamic_cast<AViewContainer*>(view)) {
        name = AReflect::name(c->getLayout().get());
    } else {
        name = AReflect::name(view);
    }

    // remove 'class '
    if (name.startsWith("class ")) {
        name = name.substr(6);
    }
    if (name.startsWith("LayoutVisitor::")) {
        name = "A" + name.substr(15);
    }
    return name;
}
