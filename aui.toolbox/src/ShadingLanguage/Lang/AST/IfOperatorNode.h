//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "INode.h"
#include "ExpressionNode.h"
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AVector.h>

class IfOperatorNode: public INode {
private:
    AArc<ExpressionNode> mCondition;
    AVector<AArc<INode>> mCode;

public:
    IfOperatorNode(const AArc<ExpressionNode>& condition, const AVector<AArc<INode>>& code) : mCondition(condition),
                                                                                        mCode(code) {}

    const AArc<ExpressionNode>& getCondition() const {
        return mCondition;
    }

    const AVector<AArc<INode>>& getCode() const {
        return mCode;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


