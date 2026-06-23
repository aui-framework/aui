//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>

#include <utility>
#include "INode.h"
#include "VariableDeclarationNode.h"

/**
 * Basic function declaration
 */
class FunctionDeclarationNode: public INode {
private:
    AString mReturnType;
    AString mName;
    AVector<AArc<VariableDeclarationNode>> mArgs;
    AVector<AArc<INode>> mCode;

public:
    FunctionDeclarationNode(AString returnType,
                            AString name,
                            AVector<AArc<VariableDeclarationNode>> args,
                            AVector<AArc<INode>> code):
                            mReturnType(std::move(returnType)),
                            mName(std::move(name)),
                            mArgs(std::move(args)),
                            mCode(std::move(code)) {}
    ~FunctionDeclarationNode() override = default;

    void acceptVisitor(INodeVisitor& v) override;

    const AString& getReturnType() const {
        return mReturnType;
    }

    const AString& getName() const {
        return mName;
    }

    const AVector<AArc<VariableDeclarationNode>>& getArgs() const {
        return mArgs;
    }

    const AVector<AArc<INode>>& getCode() const {
        return mCode;
    }
};


