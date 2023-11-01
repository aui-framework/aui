//
// Created by alex2 on 6/24/2021.
//

#include "INodeVisitor.h"
#include "ImportNode.h"

void ImportNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
