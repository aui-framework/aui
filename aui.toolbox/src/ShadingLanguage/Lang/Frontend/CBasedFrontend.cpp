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

//
// Created by Alex2772 on 3/12/2023.
//

#include "AUI/Logging/ALogger.h"
#include "AUI/Common/AMap.h"
#include "CppFrontend.h"
#include "CBasedFrontend.h"
#include "ShadingLanguage/Lang/AST/FlagDirectiveNode.h"

void CBasedFrontend::emitBinaryOperator(const AString& symbol, const BinaryOperatorNode& binaryOperator) {
    binaryOperator.getLeft()->acceptVisitor(*this);
    mShaderOutput << symbol;
    binaryOperator.getRight()->acceptVisitor(*this);
}

void CBasedFrontend::reportError(const INode& node, const AString& message) {
    ALogger::err("Frontend") << ":" << node.getLineNumber() << " " << message;
    setHasError();
}

void CBasedFrontend::visitNode(const EqualsOperatorNode& node) {
    emitBinaryOperator("==", node);
}

void CBasedFrontend::visitNode(const NotEqualsOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("!=", node);
}

void CBasedFrontend::visitNode(const ConstructorDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const LShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const RShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const MemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator(".", node);
}

void CBasedFrontend::visitNode(const StaticMemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const AssignmentOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("=", node);
}

void CBasedFrontend::visitNode(const ExplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const ImplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const FunctionDeclarationNode& node) {
    INodeVisitor::visitNode(node);

    if (node.getName() == "entry") {
        // entry function
        mEntry = node.getCode();
        return;
    }
    mDeclaredFunctions << node;

    mShaderOutput << mapType(node.getReturnType()) << " " << node.getName() << "(";
    emitFunctionDeclArguments(node);
    mShaderOutput << "){";
    visitCodeBlock(node.getCode());
    mShaderOutput << "}";
}

void CBasedFrontend::emitFunctionDeclArguments(const FunctionDeclarationNode& node, bool first) {
    for (const auto& v: node.getArgs()) {
        if (first) {
            first = false;
        } else {
            mShaderOutput << ",";
        }
        visitNode(*v);
    }
}

void CBasedFrontend::visitCodeBlock(const AVector<_<INode>>& codeBlock) {
    for (const auto& v: codeBlock) {
        v->acceptVisitor(*this);
        mShaderOutput << ";";
    }
}

void CBasedFrontend::visitNode(const LambdaNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const MethodDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const NullptrNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const IntegerNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << node.toString() << ".0";
}

void CBasedFrontend::visitNode(const FlagDirectiveNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "\n#ifdef " << node.name() << "\n"
                  << node.contents() << "\n"
                  << "\n#endif\n"
                  ;
}

void CBasedFrontend::visitNode(const OperatorCallNode& node) {
    INodeVisitor::visitNode(node);
    auto function = [&]() -> AOptional<BuiltinOrDeclaredFunction> {
        if (auto i = internalFunctions().contains(node.getCallee())) {
            mShaderOutput << i->second;
            return { i->second };
        } else {
            auto decl = mDeclaredFunctions.findIf([&](const FunctionDeclarationNode& n) { return n.getName() == node.getCallee(); });
            if (decl == nullptr) {
                reportError(node, "no such function: {}"_format(node.getCallee()));
                return std::nullopt;
            }
            mShaderOutput << node.getCallee();
            return { decl };
        }
    }();
    if (!function) {
        return;
    }

    mShaderOutput << "(";
    emitFunctionCallArguments(*function, node.getArgs(), true);
    mShaderOutput << ")";
}

void CBasedFrontend::emitFunctionCallArguments(const BuiltinOrDeclaredFunction& function, const AVector<_<ExpressionNode>>& args, bool first) {
    for (const auto& arg : args) {
        if (first) {
            first = false;
        } else {
            mShaderOutput << ",";
        }
        arg->acceptVisitor(*this);
    }
}

void CBasedFrontend::visitNode(const OperatorLiteralNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const StringNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const TemplateOperatorCallNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const TernaryOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const ThisNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const ReturnOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "return ";
    node.child()->acceptVisitor(*this);
}

void CBasedFrontend::visitNode(const LogicalNotOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const PointerDereferenceOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const PointerCreationOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const VariableDeclarationNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << mapType(node.typeName()) << " " << node.variableName();
    if (const auto& init = node.initializer()) {
        mShaderOutput << " = ";
        init->acceptVisitor(*this);
    }
}

void CBasedFrontend::visitNode(const LogicalAndOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("&&", node);
}

void CBasedFrontend::visitNode(const LogicalOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("||", node);
}

void CBasedFrontend::visitNode(const BitwiseOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("|", node);
}

void CBasedFrontend::visitNode(const ModOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("%", node);
}

void CBasedFrontend::visitNode(const BinaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "(";
    emitBinaryOperator("-", node);
    mShaderOutput << ")";
}

void CBasedFrontend::visitNode(const BinaryPlusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "(";
    emitBinaryOperator("+", node);
    mShaderOutput << ")";
}

void CBasedFrontend::visitNode(const BinaryAsteriskOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "(";
    emitBinaryOperator("*", node);
    mShaderOutput << ")";
}

void CBasedFrontend::visitNode(const BinaryDivideOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "(";
    emitBinaryOperator("/", node);
    mShaderOutput << ")";
}


void CBasedFrontend::visitNode(const UnaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "(";
    mShaderOutput << "-";
    mShaderOutput << ")";
}


void CBasedFrontend::visitNode(const IfOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const StructClassDefinition& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const GreaterOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator(">", node);
}

void CBasedFrontend::visitNode(const LessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("<", node);
}

void CBasedFrontend::visitNode(const BoolNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << AString::number(node.getValue());
}

void CBasedFrontend::visitNode(const TemplateOperatorTypenameNode& node) {
    INodeVisitor::visitNode(node);
}

void CBasedFrontend::visitNode(const ArrayAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "texture2D(";
    node.getLeft()->acceptVisitor(*this);
    mShaderOutput << ",";
    node.getRight()->acceptVisitor(*this);
    mShaderOutput << ")";

}

void CBasedFrontend::visitNode(const FloatNode& node) {
    INodeVisitor::visitNode(node);
    auto str = "{:0.9}"_format(node.getNumber());
    if (!str.contains('.')) {
        str += ".0";
    }
    mShaderOutput << str;
}

void CBasedFrontend::parseShader(const _<AST>& ast) {
    ast->visit(*this);

    if (!mEntry) {
        throw AException("no entry point in this compilation unit");
    }

    if (!mInputDefined && shaderType() == ShaderType::VERTEX) {
        throw AException("no input defined in this compilation unit");
    }

    if (!mOutputDefined && shaderType() == ShaderType::FRAGMENT) {
        throw AException("no output defined in this compilation unit");
    }
    if (!mUniformDefined) {
        visitNode(NonIndexedAttributesDeclarationNode{KeywordToken::UNIFORM, {}});
    }
    if (!mInterDefined) {
        visitNode(NonIndexedAttributesDeclarationNode{KeywordToken::INTER, {}});
    }

    emitBeforeEntryCode();
    for (const auto& c: *mEntry) {
        c->acceptVisitor(*this);
        mShaderOutput << ";";
    }
    emitAfterEntryCode();
}

void CBasedFrontend::visitNode(const IndexedAttributesDeclarationNode& node) {
    INodeVisitor::visitNode(node);
    switch (node.type()) {
        case KeywordToken::INPUT:
            if (shaderType() != ShaderType::VERTEX) {
                reportError(node, "input could be defined only for vsh");
                return;
            }
            if (mInputDefined) {
                reportError(node, "input is already defined");
            }
            mInputDefined = true;
            break;

        case KeywordToken::OUTPUT:
            if (shaderType() != ShaderType::FRAGMENT) {
                reportError(node, "output could be defined only for fsh");
                return;
            }
            if (mOutputDefined) {
                reportError(node, "output is already defined");
            }
            mOutputDefined = true;
            break;

        default:
            throw AException("internal compiler error");
    }
}
void CBasedFrontend::visitNode(const NonIndexedAttributesDeclarationNode& node) {
    INodeVisitor::visitNode(node);
    switch (node.type()) {

        case KeywordToken::UNIFORM:
            if (mUniformDefined) {
                reportError(node, "output is already defined");
            }
            mUniformDefined = true;
            break;

        case KeywordToken::INTER:
            if (shaderType() == ShaderType::IMPORTED) {
                reportError(node, "inter is not allowed for imported shaders");
            }
            if (mInterDefined) {
                reportError(node, "inter is already defined");
            }
            mInterDefined = true;
            break;

        case KeywordToken::TEXTURE:
            if (mTextureDefined) {
                reportError(node, "texture is already defined");
            }
            mTextureDefined = true;
            break;

        default:
            throw AException("internal compiler error");
    }
}

void CBasedFrontend::visitNode(const ImportNode& node) {
    INodeVisitor::visitNode(node);
    auto prevType = shaderType();
    setShaderType(ShaderType::IMPORTED);
    for (const auto& n : node.importedAst()->nodes()) {
        n->acceptVisitor(*this);
        mUniformDefined = false;
        mTextureDefined = false;
    }
    setShaderType(prevType);
}

void CBasedFrontend::visitNode(const VariableReferenceNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << node.getVariableName();
}

AString CBasedFrontend::shaderCode() {
    return mShaderOutput.str();
}

void CBasedFrontend::writeCpp(const APath& destination) {
    mNamespaceName = "aui::sl_gen::{}"_format(destination.filenameWithoutExtension().replacedAll(".", "::"));
    const auto headerPath = destination.extensionChanged("h");
    // cpp
    {
        AFileOutputStream fos(destination);
        writeCppCpp(headerPath, fos);
    }

    // header
    {

        AFileOutputStream fos(headerPath);
        writeCppHeader(fos);
    }
}

void CBasedFrontend::writeCppHeader(aui::no_escape<IOutputStream> os) {
    *os << "// This file is autogenerated by aui.toolbox. Please do not modify.\n";
    *os << "#include <AUI/SL/SL.h>\n";
    *os << "namespace " << namespaceName() << " {\n";
    emitHeaderDefinition(os);
    *os << "}\n";
}

void CBasedFrontend::writeCppCpp(const APath& headerPath, aui::no_escape<IOutputStream> os) {
    *os << "// This file is autogenerated by aui.toolbox. Please do not modify.\n";
    *os << "#define GLM_FORCE_SWIZZLE\n"; // for cpp frontend
    *os << "#include \"" << headerPath.filename() << "\"\n";
    *os << "using namespace " << namespaceName() << ";\n";
    emitCppCreateShader(os);
}

void CBasedFrontend::emitHeaderDefinition(aui::no_escape<IOutputStream> os) {
    *os << "_unique<ASLShader> createShader();\n";
}

void CBasedFrontend::emitCppCreateShader(aui::no_escape<IOutputStream> os) {

}

