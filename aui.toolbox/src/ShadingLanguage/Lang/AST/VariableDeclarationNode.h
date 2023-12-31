//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "INode.h"
#include "ExpressionNode.h"
#include "AUI/Common/SharedPtrTypes.h"

/**
 * A node describing variable declaration
 */
class VariableDeclarationNode: public ExpressionNode {
private:
    bool mIsConst;
    bool mIsStatic;
    AString mTypeName;
    AString mVariableName;
    unsigned mPointerCount;
    bool mIsReference;
    _<ExpressionNode> mInitializer;

public:
    VariableDeclarationNode(bool isConst, bool isStatic, AString typeName, AString variableName,
                            unsigned int pointerCount, bool isReference, _<ExpressionNode> initializer = nullptr) : mIsConst(isConst), mIsStatic(isStatic),
                                                                           mTypeName(std::move(typeName)),
                                                                           mVariableName(std::move(variableName)),
                                                                           mPointerCount(pointerCount),
                                                                           mIsReference(isReference),
                                                                           mInitializer(std::move(initializer)) {}

    void acceptVisitor(INodeVisitor& v) override;

    bool isConst() const {
        return mIsConst;
    }

    bool isStatic() const {
        return mIsStatic;
    }

    const AString& typeName() const {
        return mTypeName;
    }

    const AString& variableName() const {
        return mVariableName;
    }

    unsigned int pointerCount() const {
        return mPointerCount;
    }

    bool isReference() const {
        return mIsReference;
    }

    const _<ExpressionNode>& initializer() const {
        return mInitializer;
    }
};

