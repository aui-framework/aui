//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "MethodDeclarationNode.h"

class ConstructorDeclarationNode: public MethodDeclarationNode {
private:
    AVector<AArc<INode>> mInitializerList;

public:

    ConstructorDeclarationNode(const AString& returnType, const AString& name,
                               const AVector<AArc<VariableDeclarationNode>>& args, const AString& className,
                               const AVector<AArc<INode>>& initializerList,
                               const AVector<AArc<INode>>& code) : MethodDeclarationNode(returnType, name, args,
                                                                                                 className, code),
                                                                           mInitializerList(initializerList) {}

    const AVector<AArc<INode>>& getInitializerList() const {
        return mInitializerList;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


