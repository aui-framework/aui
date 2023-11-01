//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "ExpressionNode.h"

class VariableReferenceNode: public ExpressionNode {
private:
    AString mVariableName;

public:
    VariableReferenceNode(const AString& variableName) : mVariableName(variableName) {}

    void acceptVisitor(INodeVisitor& v) override;

    [[nodiscard]]
    const AString& getVariableName() const {
        return mVariableName;
    }
};


