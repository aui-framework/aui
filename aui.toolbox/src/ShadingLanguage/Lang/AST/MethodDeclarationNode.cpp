//
// Created by alex2 on 6/12/2021.
//

#include "MethodDeclarationNode.h"
#include "INodeVisitor.h"

void MethodDeclarationNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
