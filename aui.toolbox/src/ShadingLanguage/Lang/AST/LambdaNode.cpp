//
// Created by alex2 on 6/14/2021.
//

#include "LambdaNode.h"
#include "INodeVisitor.h"

void LambdaNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
