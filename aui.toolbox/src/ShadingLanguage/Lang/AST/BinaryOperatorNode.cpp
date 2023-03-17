//
// Created by alex2 on 6/13/2021.
//

#include "BinaryOperatorNode.h"
#include "INodeVisitor.h"

void LShiftOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void RShiftOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void MemberAccessOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void AssignmentOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void BitwiseOrOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void LogicalOrOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void ModOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void BinaryMinusOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void BinaryPlusOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void LogicalAndOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void EqualsOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void NotEqualsOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void GreaterOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void LessOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void StaticMemberAccessOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void ArrayAccessOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void BinaryAsteriskOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}

void BinaryDivideOperatorNode::acceptVisitor(INodeVisitor& v) {
    v.visitNode(*this);
}
