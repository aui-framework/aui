// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "IFrontend.h"
#include "AUI/IO/AStringStream.h"

class CBasedFrontend: public IFrontend, public INodeVisitor {
public:
    using INodeVisitor::visitNode;

    using BuiltinOrDeclaredFunction = std::variant<AString, const FunctionDeclarationNode*>;

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
    void visitNode(const BinaryAsteriskOperatorNode& node) override;
    void visitNode(const BinaryDivideOperatorNode& node) override;
    void visitNode(const UnaryMinusOperatorNode& node) override;
    void visitNode(const IfOperatorNode& node) override;
    void visitNode(const StructClassDefinition& node) override;
    void visitNode(const GreaterOperatorNode& node) override;
    void visitNode(const LessOperatorNode& node) override;
    void visitNode(const BoolNode& node) override;
    void visitNode(const TemplateOperatorTypenameNode& node) override;
    void visitNode(const ArrayAccessOperatorNode& node) override;
    void visitNode(const FloatNode& node) override;
    void visitNode(const IndexedAttributesDeclarationNode& node) override;
    void visitNode(const NonIndexedAttributesDeclarationNode& node) override;
    void visitNode(const ImportNode& node) override;
    void visitNode(const FlagDirectiveNode& node) override;

    AString shaderCode() override;

    ~CBasedFrontend() override = default;

    void writeCpp(const APath& destination) override;

    void visitCodeBlock(const AVector<_<INode>>& codeBlock);

    void writeCppHeader(aui::no_escape<IOutputStream> os);
    void writeCppCpp(const APath& headerPath, aui::no_escape<IOutputStream> os);
protected:
    virtual void emitBinaryOperator(const AString& symbol, const BinaryOperatorNode& binaryOperator);
    void reportError(const INode& node, const AString& message);
    virtual void emitHeaderDefinition(aui::no_escape<IOutputStream> os);
    virtual void emitCppCreateShader(aui::no_escape<IOutputStream> os);
    virtual void emitFunctionDeclArguments(const FunctionDeclarationNode& node, bool first = true);
    virtual void emitFunctionCallArguments(const BuiltinOrDeclaredFunction& function, const AVector<_<ExpressionNode>>& args, bool first);

    virtual AString mapType(const AString& type) = 0;
    virtual void emitBeforeEntryCode() = 0;
    virtual void emitAfterEntryCode() = 0;

    bool mInputDefined = false;
    bool mOutputDefined = false;
    bool mUniformDefined = false;
    bool mInterDefined = false;
    bool mTextureDefined = false;

    AOptional<AVector<_<INode>>> mEntry;
    AStringStream mShaderOutput;

    const AString& namespaceName() const {
        return mNamespaceName;
    }

    virtual const AMap<AString, AString>& internalFunctions() = 0;

private:
    AString mNamespaceName;
    AVector<FunctionDeclarationNode> mDeclaredFunctions;
};
