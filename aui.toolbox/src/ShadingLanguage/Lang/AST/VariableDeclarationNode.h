//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "INode.h"
#include "ExpressionNode.h"

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

public:
    VariableDeclarationNode(bool isConst, bool isStatic, const AString& typeName, const AString& variableName,
                            unsigned int pointerCount, bool isReference) : mIsConst(isConst), mIsStatic(isStatic),
                                                                           mTypeName(typeName),
                                                                           mVariableName(variableName),
                                                                           mPointerCount(pointerCount),
                                                                           mIsReference(isReference) {}

    void acceptVisitor(INodeVisitor& v) override;

    bool isConst() const {
        return mIsConst;
    }

    bool isStatic() const {
        return mIsStatic;
    }

    const AString& getTypeName() const {
        return mTypeName;
    }

    const AString& getVariableName() const {
        return mVariableName;
    }

    unsigned int getPointerCount() const {
        return mPointerCount;
    }

    bool isReference() const {
        return mIsReference;
    }
};

