//
// Created by alex2 on 6/24/2021.
//

#pragma once

#include "AUI/Common/AString.h"
#include "ExpressionNode.h"

class FlagDirectiveNode: public INode {
public:
    FlagDirectiveNode(AString name, AString contents) : mName(std::move(name)), mContents(std::move(contents)) {}

    [[nodiscard]]
    AString name() const noexcept {
        return mName;
    }
    
    [[nodiscard]]
    AString contents() const noexcept {
        return mContents;
    }

    void acceptVisitor(INodeVisitor& v) override;

private:
    AString mName;
    AString mContents;
};


