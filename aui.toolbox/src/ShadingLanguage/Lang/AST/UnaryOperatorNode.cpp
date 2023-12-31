//
// Created by alex2 on 6/15/2021.
//

#include "UnaryOperatorNode.h"
#include "INodeVisitor.h"


void ReturnOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void LogicalNotOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void PointerDereferenceOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void PointerCreationOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void UnaryMinusOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
