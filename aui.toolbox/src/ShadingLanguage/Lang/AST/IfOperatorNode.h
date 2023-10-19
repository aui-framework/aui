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
    _<ExpressionNode> mCondition;
    AVector<_<INode>> mCode;

public:
    IfOperatorNode(const _<ExpressionNode>& condition, const AVector<_<INode>>& code) : mCondition(condition),
                                                                                        mCode(code) {}

    const _<ExpressionNode>& getCondition() const {
        return mCondition;
    }

    const AVector<_<INode>>& getCode() const {
        return mCode;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


