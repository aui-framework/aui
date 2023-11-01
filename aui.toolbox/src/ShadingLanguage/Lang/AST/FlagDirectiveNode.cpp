//
// Created by alex2 on 6/24/2021.
//

#include "FlagDirectiveNode.h"
#include "INodeVisitor.h"

void FlagDirectiveNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
