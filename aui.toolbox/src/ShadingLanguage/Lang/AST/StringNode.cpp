//
// Created by alex2 on 6/12/2021.
//

#include "StringNode.h"
#include "INodeVisitor.h"

void StringNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
