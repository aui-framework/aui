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


#include "AUI/IO/IOutputStream.h"
#include "ShadingLanguage/Lang/AST/AST.h"
#include "CBasedFrontend.h"
#include "ShadingLanguage/Lang/AST/INodeVisitor.h"
#include "IFronted.h"

class CppFrontend: public CBasedFrontend {
public:
    using CBasedFrontend::CBasedFrontend;

    void visitNode(const IndexedAttributesDeclarationNode& node) override;
    void visitNode(const NonIndexedAttributesDeclarationNode& node) override;
    void visitNode(const VariableReferenceNode& node) override;
    void visitNode(const MemberAccessOperatorNode& node) override;
    void visitNode(const ArrayAccessOperatorNode& node) override;
    void visitNode(const FunctionDeclarationNode& node) override;
    void visitNode(const FloatNode& node) override;
    void visitNode(const IntegerNode& node) override;

protected:
    AString mapType(const AString& type) override;
    void emitBeforeEntryCode() override;
    void emitAfterEntryCode() override;

    void emitHeaderDefinition(aui::no_escape<IOutputStream> os) override;

    void emitCppCreateShader(aui::no_escape<IOutputStream> os) override;

    const AMap<AString, AString>& internalFunctions() override;

    void emitFunctionDeclArguments(const FunctionDeclarationNode& node, bool first) override;

    void emitFunctionCallArguments(const BuiltinOrDeclaredFunction& function, const AVector<_<ExpressionNode>>& args, bool first) override;

private:
    AStringStream mHeaderOutput;
    AVector<NonIndexedAttributesDeclarationNode> mHeaderNonIndexedAttributesDeclarations;

    void emitAttributeKeyword(KeywordToken::Type type);
    void emitAttributeDeclarationField(_<VariableDeclarationNode> node);
};
