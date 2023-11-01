//
// Created by alex2 on 6/14/2021.
//

#pragma once

#include "ExpressionNode.h"

class ThisNode: public ExpressionNode {
public:
    void acceptVisitor(INodeVisitor& v) override;
};


