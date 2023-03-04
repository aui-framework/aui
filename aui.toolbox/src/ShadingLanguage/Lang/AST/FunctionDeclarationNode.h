//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include <AUI/Common/AVector.h>
#include "INode.h"
#include "VariableDeclarationNode.h"

/**
 * Basic function declaration
 */
class FunctionDeclarationNode: public INode {
private:
    AString mReturnType;
    AString mName;
    AVector<_<VariableDeclarationNode>> mArgs;
    AVector<_<INode>> mCode;

public:
    FunctionDeclarationNode(const AString& returnType, const AString& name,
                            const AVector<_<VariableDeclarationNode>>& args,
                            const AVector<_<INode>>& code) : mReturnType(returnType), mName(name),
                                                                               mArgs(args), mCode(code) {}
    ~FunctionDeclarationNode() override = default;

    void acceptVisitor(INodeVisitor& v) override;

    const AString& getReturnType() const {
        return mReturnType;
    }

    const AString& getName() const {
        return mName;
    }

    const AVector<_<VariableDeclarationNode>>& getArgs() const {
        return mArgs;
    }

    const AVector<_<INode>>& getCode() const {
        return mCode;
    }
};


