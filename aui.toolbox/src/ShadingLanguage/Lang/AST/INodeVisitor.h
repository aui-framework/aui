//
// Created by alex2 on 6/15/2021.
//

#pragma once

#include "ConstructorDeclarationNode.h"
#include "BinaryOperatorNode.h"
#include "ExplicitInitializerListCtorNode.h"
#include "ImplicitInitializerListCtorNode.h"
#include "FunctionDeclarationNode.h"
#include "LambdaNode.h"
#include "MethodDeclarationNode.h"
#include "NullptrNode.h"
#include "IntegerNode.h"
#include "OperatorCallNode.h"
#include "OperatorLiteralNode.h"
#include "StringNode.h"
#include "TemplateOperatorCallNode.h"
#include "TernaryOperatorNode.h"
#include "ThisNode.h"
#include "UnaryOperatorNode.h"
#include "VariableDeclarationNode.h"
#include "VariableReferenceNode.h"
#include "IfOperatorNode.h"
#include "StructClassDefinition.h"
#include "BoolNode.h"
#include "TemplateOperatorTypenameNode.h"
#include "FloatNode.h"
#include "AUI/Reflect/AClass.h"
#include "IndexedAttributesDeclarationNode.h"
#include "NonIndexedAttributesDeclarationNode.h"

class INodeVisitor {
public:
    // compare operators
    virtual void visitNode(const EqualsOperatorNode& node) {};
    virtual void visitNode(const NotEqualsOperatorNode& node) {};

    // some shit
    virtual void visitNode(const ConstructorDeclarationNode& node) {};
    virtual void visitNode(const LShiftOperatorNode& node) {};
    virtual void visitNode(const RShiftOperatorNode& node) {};
    virtual void visitNode(const MemberAccessOperatorNode& node) {};
    virtual void visitNode(const StaticMemberAccessOperatorNode& node) {};
    virtual void visitNode(const AssignmentOperatorNode& node) {};
    virtual void visitNode(const ExplicitInitializerListCtorNode& node) {};
    virtual void visitNode(const ImplicitInitializerListCtorNode& node) {};
    virtual void visitNode(const FunctionDeclarationNode& node) {};
    virtual void visitNode(const LambdaNode& node) {};
    virtual void visitNode(const MethodDeclarationNode& node) {};
    virtual void visitNode(const NullptrNode& node) {};
    virtual void visitNode(const IntegerNode& node) {};
    virtual void visitNode(const OperatorCallNode& node) {};
    virtual void visitNode(const OperatorLiteralNode& node) {};
    virtual void visitNode(const StringNode& node) {};
    virtual void visitNode(const TemplateOperatorCallNode& node) {};
    virtual void visitNode(const TernaryOperatorNode& node) {};
    virtual void visitNode(const ThisNode& node) {};
    virtual void visitNode(const ReturnOperatorNode& node) {};
    virtual void visitNode(const LogicalNotOperatorNode& node) {};
    virtual void visitNode(const PointerDereferenceOperatorNode& node) {};
    virtual void visitNode(const PointerCreationOperatorNode& node) {};
    virtual void visitNode(const VariableDeclarationNode& node) {};
    virtual void visitNode(const VariableReferenceNode& node) {};
    virtual void visitNode(const LogicalAndOperatorNode& node) {};
    virtual void visitNode(const LogicalOrOperatorNode& node) {};
    virtual void visitNode(const BitwiseOrOperatorNode& node) {};
    virtual void visitNode(const ModOperatorNode& node) {};
    virtual void visitNode(const BinaryMinusOperatorNode& node) {};
    virtual void visitNode(const BinaryPlusOperatorNode& node) {};
    virtual void visitNode(const UnaryMinusOperatorNode& node) {};
    virtual void visitNode(const IfOperatorNode& node) {};
    virtual void visitNode(const StructClassDefinition& node) {};
    virtual void visitNode(const GreaterOperatorNode& node) {};
    virtual void visitNode(const LessOperatorNode& node) {};
    virtual void visitNode(const BoolNode& node) {};
    virtual void visitNode(const TemplateOperatorTypenameNode& node) {};
    virtual void visitNode(const ArrayAccessOperatorNode& node) {};
    virtual void visitNode(const BinaryAsteriskOperatorNode& node) {};
    virtual void visitNode(const FloatNode& node) {};
    virtual void visitNode(const IndexedAttributesDeclarationNode& node) {};
    virtual void visitNode(const NonIndexedAttributesDeclarationNode& node) {};

    template<class T>
    static _<T> assume(const _<INode>& n) {
        if (auto c = _cast<T>(n)) {
            return c;
        }
        throw AException("assume failed: expected " + AClass<T>::name() + ", got " + AReflect::name(n.get()));
    }

};