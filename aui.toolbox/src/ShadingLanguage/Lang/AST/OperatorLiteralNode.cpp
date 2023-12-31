//
// Created by alex2 on 6/12/2021.
//

#include "OperatorLiteralNode.h"
#include "INodeVisitor.h"

void OperatorLiteralNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

AString OperatorLiteralNode::toString() const {
    return IStringable::toString(mChild) + mLiteralName;
}
