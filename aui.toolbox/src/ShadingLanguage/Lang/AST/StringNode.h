//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "ExpressionNode.h"

class StringNode: public ExpressionNode {
private:
    AString mString;

public:
    StringNode(const AString& string) : mString(string) {}

    const AString& getString() const {
        return mString;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


