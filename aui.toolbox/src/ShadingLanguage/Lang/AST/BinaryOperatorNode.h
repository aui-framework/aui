//
// Created by alex2 on 6/13/2021.
//

#pragma once


#include "ExpressionNode.h"
#include <AUI/Common/SharedPtrTypes.h>

class BinaryOperatorNode: public ExpressionNode {
    friend class Parser;
private:
    _<ExpressionNode> mLeft;
    _<ExpressionNode> mRight;

public:
    BinaryOperatorNode(_<ExpressionNode> left, const _<ExpressionNode> right) : mLeft(left), mRight(right) {}


    const _<ExpressionNode>& getLeft() const {
        return mLeft;
    }

    const _<ExpressionNode>& getRight() const {
        return mRight;
    }
};


class LShiftOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class RShiftOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class MemberAccessOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};
class StaticMemberAccessOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class AssignmentOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class LogicalOrOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class LogicalAndOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class BitwiseOrOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};


class ModOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class BinaryPlusOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class BinaryMinusOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class BinaryAsteriskOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class BinaryDivideOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class EqualsOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class NotEqualsOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class GreaterOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class LessOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};

class ArrayAccessOperatorNode: public BinaryOperatorNode {
public:
    using BinaryOperatorNode::BinaryOperatorNode;

    void acceptVisitor(INodeVisitor& v) override;
};
