//
// Created by alex2 on 6/12/2021.
//

#include "FunctionDeclarationNode.h"
#include "INodeVisitor.h"

void FunctionDeclarationNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
