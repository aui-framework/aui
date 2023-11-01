//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "FunctionDeclarationNode.h"

/**
 * Class member method declaration
 */
class MethodDeclarationNode: public FunctionDeclarationNode {
private:
    AString mClassName;

public:
    MethodDeclarationNode(const AString& returnType, const AString& name,
                          const AVector<_<VariableDeclarationNode>>& args, const AString& className,
                          const AVector<_<INode>>& code)
            : FunctionDeclarationNode(returnType, name, args, code), mClassName(className) {}

    const AString& getClassName() const {
        return mClassName;
    }

    void acceptVisitor(INodeVisitor& v) override;

};


