//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "INode.h"
#include <AUI/Common/SharedPtr.h>

class AST {
private:
    AVector<AArc<INode>> mNodes;

public:
    AST(AVector<AArc<INode>> nodes) : mNodes(std::move(nodes)) {}

    const AVector<AArc<INode>>& nodes() const {
        return mNodes;
    }

    void visit(INodeVisitor& visitor);

};


