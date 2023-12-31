//
// Created by alex2 on 6/12/2021.
//

#include "IntegerNode.h"
#include "INodeVisitor.h"

void IntegerNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

AString IntegerNode::toString() const {
    if (mIsHex) {
        return "0x" + AString::numberHex(mNumber);
    } else {
        return AString::number(mNumber);
    }
}
