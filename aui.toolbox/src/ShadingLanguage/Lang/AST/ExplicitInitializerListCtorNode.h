//
// Created by alex2 on 6/13/2021.
//

#pragma once

#include "ExpressionNode.h"
#include <AUI/Common/AString.h>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AVector.h>

class ExplicitInitializerListCtorNode: public ExpressionNode {
private:
    AString mClassName;
    AVector<_<ExpressionNode>> mArgs;

public:
    ExplicitInitializerListCtorNode(const AString& className, const AVector<_<ExpressionNode>>& args) : mClassName(className),
                                                                                                 mArgs(args) {}

    void acceptVisitor(INodeVisitor& v) override;

    const AString& getClassName() const {
        return mClassName;
    }

    const AVector<_<ExpressionNode>>& getArgs() const {
        return mArgs;
    }
};


