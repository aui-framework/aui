//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "INode.h"
#include "IndexedAttributesDeclarationNode.h"
#include <AUI/Common/SharedPtr.h>

class AST {
private:
    AVector<_<INode>> mNodes;

public:
    AST(const AVector<_<INode>>& nodes) : mNodes(nodes) {}

    void visit(INodeVisitor& visitor);

};


