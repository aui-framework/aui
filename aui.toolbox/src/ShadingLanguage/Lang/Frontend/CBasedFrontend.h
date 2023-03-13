// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "ShadingLanguage/Lang/AST/INodeVisitor.h"
#include "AUI/IO/IOutputStream.h"
#include "ShadingLanguage/Lang/AST/AST.h"
#include "IFronted.h"
#include "AUI/IO/AStringStream.h"

class CBasedFrontend: public IFrontend, public INodeVisitor {
public:
    using INodeVisitor::visitNode;

    void parseShader(const _<AST>& ast) override;

    void visitNode(const EqualsOperatorNode& node) override;
    void visitNode(const NotEqualsOperatorNode& node) override;
    void visitNode(const ConstructorDeclarationNode& node) override;
    void visitNode(const LShiftOperatorNode& node) override;
    void visitNode(const RShiftOperatorNode& node) override;
    void visitNode(const MemberAccessOperatorNode& node) override;
    void visitNode(const StaticMemberAccessOperatorNode& node) override;
    void visitNode(const AssignmentOperatorNode& node) override;
    void visitNode(const ExplicitInitializerListCtorNode& node) override;
    void visitNode(const ImplicitInitializerListCtorNode& node) override;
    void visitNode(const FunctionDeclarationNode& node) override;
    void visitNode(const LambdaNode& node) override;
    void visitNode(const MethodDeclarationNode& node) override;
    void visitNode(const NullptrNode& node) override;
    void visitNode(const IntegerNode& node) override;
    void visitNode(const OperatorCallNode& node) override;
    void visitNode(const OperatorLiteralNode& node) override;
    void visitNode(const StringNode& node) override;
    void visitNode(const TemplateOperatorCallNode& node) override;
    void visitNode(const TernaryOperatorNode& node) override;
    void visitNode(const ThisNode& node) override;
    void visitNode(const ReturnOperatorNode& node) override;
    void visitNode(const LogicalNotOperatorNode& node) override;
    void visitNode(const PointerDereferenceOperatorNode& node) override;
    void visitNode(const PointerCreationOperatorNode& node) override;
    void visitNode(const VariableDeclarationNode& node) override;
    void visitNode(const VariableReferenceNode& node) override;
    void visitNode(const LogicalAndOperatorNode& node) override;
    void visitNode(const LogicalOrOperatorNode& node) override;
    void visitNode(const BitwiseOrOperatorNode& node) override;
    void visitNode(const ModOperatorNode& node) override;
    void visitNode(const BinaryMinusOperatorNode& node) override;
    void visitNode(const BinaryPlusOperatorNode& node) override;
    void visitNode(const UnaryMinusOperatorNode& node) override;
    void visitNode(const IfOperatorNode& node) override;
    void visitNode(const StructClassDefinition& node) override;
    void visitNode(const GreaterOperatorNode& node) override;
    void visitNode(const LessOperatorNode& node) override;
    void visitNode(const BoolNode& node) override;
    void visitNode(const TemplateOperatorTypenameNode& node) override;
    void visitNode(const ArrayAccessOperatorNode& node) override;
    void visitNode(const FloatNode& node) override;
    void visitNode(const IndexedAttributeDeclarationNode& node) override;

    AString shaderCode() override;

protected:
    void emitBinaryOperator(const AString& symbol, const BinaryOperatorNode& binaryOperator);
    void reportError(const INode& node, const AString& message);
    bool isVertex();

    virtual AString mapType(const AString& type) = 0;
    virtual void emitBeforeEntryCode() = 0;
    virtual void emitAfterEntryCode() = 0;

    bool mInputDefined = false;
    bool mOutputDefined = false;
    bool mUniformDefined = false;

    AOptional<AVector<_<INode>>> mEntry;
    AStringStream mShaderOutput;
};
