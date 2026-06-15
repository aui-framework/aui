//
// Created by alex2 on 6/14/2021.
//

#pragma once


#include "ExpressionNode.h"
#include "VariableDeclarationNode.h"
#include <AUI/Common/AVector.h>

class LambdaNode: public ExpressionNode {
private:
    AVector<AArc<VariableDeclarationNode>> mArgs;
    AVector<AArc<INode>> mCode;

public:
    LambdaNode(const AVector<AArc<VariableDeclarationNode>>& args, const AVector<AArc<INode>>& code) : mArgs(args),
                                                                                                 mCode(code) {}

    void acceptVisitor(INodeVisitor& v) override;
};


