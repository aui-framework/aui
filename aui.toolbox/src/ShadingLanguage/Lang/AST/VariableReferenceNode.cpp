//
// Created by alex2 on 6/12/2021.
//

#include "VariableReferenceNode.h"
#include "INodeVisitor.h"

void VariableReferenceNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
