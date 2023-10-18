//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "MethodDeclarationNode.h"

class ConstructorDeclarationNode: public MethodDeclarationNode {
private:
    AVector<_<INode>> mInitializerList;

public:

    ConstructorDeclarationNode(const AString& returnType, const AString& name,
                               const AVector<_<VariableDeclarationNode>>& args, const AString& className,
                               const AVector<_<INode>>& initializerList,
                               const AVector<_<INode>>& code) : MethodDeclarationNode(returnType, name, args,
                                                                                                 className, code),
                                                                           mInitializerList(initializerList) {}

    const AVector<_<INode>>& getInitializerList() const {
        return mInitializerList;
    }

    void acceptVisitor(INodeVisitor& v) override;
};


