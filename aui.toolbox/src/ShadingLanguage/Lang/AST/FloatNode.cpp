//
// Created by alex2 on 6/29/2021.
//

#include "FloatNode.h"
#include "INodeVisitor.h"

void FloatNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

AString FloatNode::toString() const {
    return AString::number(mNumber);
}
