//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "ExpressionNode.h"
#include <cstdint>
#include <AUI/Common/IStringable.h>

class IntegerNode: public ExpressionNode, public IStringable {
private:
    int64_t mNumber;
    bool mIsHex;

public:
    IntegerNode(int64_t number, bool isHex) : mNumber(number), mIsHex(isHex) {}

    int64_t getNumber() const {
        return mNumber;
    }

    bool isHex() const {
        return mIsHex;
    }

    void acceptVisitor(INodeVisitor& v) override;

    AString toString() const override;
};


