//
// Created by alex2 on 6/15/2021.
//

#pragma once


#include "ExpressionNode.h"
#include <AUI/Common/SharedPtrTypes.h>

class TernaryOperatorNode: public ExpressionNode {
private:
    _<ExpressionNode> mCondition;
    _<ExpressionNode> mOnTrue;
    _<ExpressionNode> mOnFalse;

public:

    TernaryOperatorNode(const _<ExpressionNode>& condition, const _<ExpressionNode>& onTrue,
                        const _<ExpressionNode>& onFalse) : mCondition(condition), mOnTrue(onTrue), mOnFalse(onFalse) {}

    void acceptVisitor(INodeVisitor& v) override;

    const _<ExpressionNode>& getCondition() const {
        return mCondition;
    }

    const _<ExpressionNode>& getOnTrue() const {
        return mOnTrue;
    }

    const _<ExpressionNode>& getOnFalse() const {
        return mOnFalse;
    }
};


