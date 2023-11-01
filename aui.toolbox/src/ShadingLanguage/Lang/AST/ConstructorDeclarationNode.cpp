//
// Created by alex2 on 6/12/2021.
//

#include "ConstructorDeclarationNode.h"
#include "INodeVisitor.h"

void ConstructorDeclarationNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
