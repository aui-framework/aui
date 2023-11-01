//
// Created by alex2 on 6/15/2021.
//

#include <AUI/Common/AStringVector.h>
#include "ImplicitInitializerListCtorNode.h"
#include "INodeVisitor.h"

void ImplicitInitializerListCtorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

AString ImplicitInitializerListCtorNode::toString() const {
    AStringVector cv;
    for (auto& e : mElements) {
        cv << IStringable::toString(e);
    }
    return "{" + cv.join(", ") + "}";
}
