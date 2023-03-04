//
// Created by alex2 on 6/13/2021.
//

#pragma once

#include "OperatorCallNode.h"

class TemplateOperatorCallNode: public OperatorCallNode {
private:
    AString mTemplateArg;

public:
    TemplateOperatorCallNode(const AString& callee, const AVector<_<ExpressionNode>>& args, const AString& templateArg)
            : OperatorCallNode(callee, args), mTemplateArg(templateArg) {}

    const AString& getTemplateArg() const {
        return mTemplateArg;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


