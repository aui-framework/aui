//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "INode.h"
#include <AUI/Common/SharedPtr.h>

class AST {
private:
    AVector<_<INode>> mNodes;

public:
    AST(AVector<_<INode>> nodes) : mNodes(std::move(nodes)) {}

    const AVector<_<INode>>& nodes() const {
        return mNodes;
    }

    void visit(INodeVisitor& visitor);

};


