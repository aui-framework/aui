#pragma once

#include "ExpressionNode.h"
#include <cstdint>
#include <AUI/Common/IStringable.h>

class FloatNode: public ExpressionNode, public IStringable {
private:
    double mNumber;

public:
    FloatNode(double number) : mNumber(number) {}

    double getNumber() const {
        return mNumber;
    }

    void acceptVisitor(INodeVisitor& v) override;

    AString toString() const override;
};
