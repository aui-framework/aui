//
// Created by alex2 on 6/15/2021.
//

#pragma once

#include <AUI/Common/SharedPtrTypes.h>
#include "ExpressionNode.h"

class UnaryOperatorNode: public ExpressionNode {
private:
    _<ExpressionNode> mChild;

public:
    UnaryOperatorNode(const _<ExpressionNode>& child) : mChild(child) {}
};


class ReturnOperatorNode: public UnaryOperatorNode {
public:
    using UnaryOperatorNode::UnaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};


class LogicalNotOperatorNode: public UnaryOperatorNode {
public:
    using UnaryOperatorNode::UnaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};


class PointerDereferenceOperatorNode: public UnaryOperatorNode {
public:
    using UnaryOperatorNode::UnaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class PointerCreationOperatorNode: public UnaryOperatorNode {
public:
    using UnaryOperatorNode::UnaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};




class UnaryMinusOperatorNode: public UnaryOperatorNode {
public:
    using UnaryOperatorNode::UnaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};


