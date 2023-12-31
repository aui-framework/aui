//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "INode.h"

class StructClassDefinition: public INode {
private:
    AString mName;
    AVector<_<INode>> mNodes;

public:
    StructClassDefinition(const AString& name, const AVector<_<INode>>& nodes) : mName(name), mNodes(nodes) {}

    void acceptVisitor(INodeVisitor& v) override;

    const AString& getName() const {
        return mName;
    }

    const AVector<_<INode>>& getNodes() const {
        return mNodes;
    }
};


