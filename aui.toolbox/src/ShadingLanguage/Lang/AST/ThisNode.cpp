//
// Created by alex2 on 6/14/2021.
//

#include "ThisNode.h"
#include "INodeVisitor.h"

void ThisNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
