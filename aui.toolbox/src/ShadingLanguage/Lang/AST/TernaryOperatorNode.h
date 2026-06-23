//
// Created by alex2 on 6/15/2021.
//

#pragma once


#include "ExpressionNode.h"
#include <AUI/Common/SharedPtrTypes.h>

class TernaryOperatorNode: public ExpressionNode {
private:
    AArc<ExpressionNode> mCondition;
    AArc<ExpressionNode> mOnTrue;
    AArc<ExpressionNode> mOnFalse;

public:

    TernaryOperatorNode(const AArc<ExpressionNode>& condition, const AArc<ExpressionNode>& onTrue,
                        const AArc<ExpressionNode>& onFalse) : mCondition(condition), mOnTrue(onTrue), mOnFalse(onFalse) {}

    void acceptVisitor(INodeVisitor& v) override;

    const AArc<ExpressionNode>& getCondition() const {
        return mCondition;
    }

    const AArc<ExpressionNode>& getOnTrue() const {
        return mOnTrue;
    }

    const AArc<ExpressionNode>& getOnFalse() const {
        return mOnFalse;
    }
};


