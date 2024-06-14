/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once


#include "AUI/IO/IOutputStream.h"
#include "ShadingLanguage/Lang/AST/AST.h"
#include "CBasedFrontend.h"
#include "ShadingLanguage/Lang/AST/INodeVisitor.h"
#include "IFrontend.h"

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
