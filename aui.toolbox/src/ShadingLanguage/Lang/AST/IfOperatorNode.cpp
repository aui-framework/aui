//
// Created by alex2 on 6/17/2021.
//

#include "IfOperatorNode.h"
#include "INodeVisitor.h"

void IfOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
