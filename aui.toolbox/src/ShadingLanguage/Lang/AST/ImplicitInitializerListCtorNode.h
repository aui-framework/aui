//
// Created by alex2 on 6/15/2021.
//

#pragma once


#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/IStringable.h>
#include "ExpressionNode.h"

class ImplicitInitializerListCtorNode: public ExpressionNode, public IStringable {
private:
    AVector<_<ExpressionNode>> mElements;

public:
    ImplicitInitializerListCtorNode(const AVector<_<ExpressionNode>>& elements) : mElements(elements) {}

    void acceptVisitor(INodeVisitor& v) override;

    [[nodiscard]] const AVector<_<ExpressionNode>>& getElements() const {
        return mElements;
    }

    AString toString() const override;
};


