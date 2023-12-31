//
// Created by alex2 on 6/14/2021.
//

#include "NullptrNode.h"
#include "INodeVisitor.h"

void NullptrNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
