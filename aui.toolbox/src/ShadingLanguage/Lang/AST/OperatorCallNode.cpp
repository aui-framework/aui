//
// Created by alex2 on 6/12/2021.
//

#include "OperatorCallNode.h"
#include "INodeVisitor.h"

void OperatorCallNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
