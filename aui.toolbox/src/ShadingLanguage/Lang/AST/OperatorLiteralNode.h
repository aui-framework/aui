//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/IStringable.h>
#include "ExpressionNode.h"

class OperatorLiteralNode: public ExpressionNode, public IStringable {
private:
    AString mLiteralName;
    _<ExpressionNode> mChild;

public:
    OperatorLiteralNode(const AString& literalName, const _<ExpressionNode>& child) : mLiteralName(literalName),
                                                                                      mChild(child) {}

    void acceptVisitor(INodeVisitor& v) override;

    AString toString() const override;

    const AString& getLiteralName() const {
        return mLiteralName;
    }

    const _<ExpressionNode>& getChild() const {
        return mChild;
    }
};


