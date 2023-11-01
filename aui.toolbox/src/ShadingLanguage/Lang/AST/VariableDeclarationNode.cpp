//
// Created by alex2 on 6/15/2021.
//

#include "VariableDeclarationNode.h"
#include "INodeVisitor.h"


void VariableDeclarationNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
