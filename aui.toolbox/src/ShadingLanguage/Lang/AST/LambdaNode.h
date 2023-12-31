//
// Created by alex2 on 6/14/2021.
//

#pragma once


#include "ExpressionNode.h"
#include "VariableDeclarationNode.h"
#include <AUI/Common/AVector.h>

class LambdaNode: public ExpressionNode {
private:
    AVector<_<VariableDeclarationNode>> mArgs;
    AVector<_<INode>> mCode;

public:
    LambdaNode(const AVector<_<VariableDeclarationNode>>& args, const AVector<_<INode>>& code) : mArgs(args),
                                                                                                 mCode(code) {}

    void acceptVisitor(INodeVisitor& v) override;
};


