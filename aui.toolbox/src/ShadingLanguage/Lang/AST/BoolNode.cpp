//
// Created by alex2 on 6/24/2021.
//

#include "BoolNode.h"
#include "INodeVisitor.h"

void BoolNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
