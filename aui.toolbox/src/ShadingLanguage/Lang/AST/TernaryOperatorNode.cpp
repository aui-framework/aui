//
// Created by alex2 on 6/15/2021.
//

#include "TernaryOperatorNode.h"
#include "INodeVisitor.h"

void TernaryOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
