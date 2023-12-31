//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include "ExpressionNode.h"

class BoolNode: public ExpressionNode {
private:
    bool mValue;

public:
    BoolNode(bool value) : mValue(value) {}

    [[nodiscard]]
    bool getValue() const {
        return mValue;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


