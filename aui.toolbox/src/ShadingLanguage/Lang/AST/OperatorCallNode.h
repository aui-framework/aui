//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "ExpressionNode.h"

class OperatorCallNode: public ExpressionNode {
private:
    AString mCallee;
    AVector<_<ExpressionNode>> mArgs;

public:
    OperatorCallNode(const AString& callee, const AVector<_<ExpressionNode>>& args) : mCallee(callee), mArgs(args) {}

    void acceptVisitor(INodeVisitor& v) override;

    const AString& getCallee() const {
        return mCallee;
    }

    const AVector<_<ExpressionNode>>& getArgs() const {
        return mArgs;
    }
};


