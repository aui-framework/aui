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

//
// Created by Alex2772 on 3/12/2023.
//

#include "AUI/Logging/ALogger.h"
#include "AUI/Common/AMap.h"
#include "CppFrontend.h"
#include "CBasedFrontend.h"

void CBasedFrontend::emitBinaryOperator(const AString& symbol, const BinaryOperatorNode& binaryOperator) {
    binaryOperator.getLeft()->acceptVisitor(*this);
    mShaderOutput << symbol;
    binaryOperator.getRight()->acceptVisitor(*this);
}

void CBasedFrontend::reportError(const INode& node, const AString& message) {
    ALogger::err("Frontend") << ":" << node.getLineNumber() << " " << message;
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
    mShaderOutput << mapType(node.getReturnType()) << " " << node.getName() << "(";
    bool first = true;
    for (const auto& v: node.getArgs()) {
        if (first) {
            first = false;
        } else {
            mShaderOutput << ",";
        }
        visitNode(*v);
    }
    mShaderOutput << "){";
    for (const auto& v: node.getCode()) {
        v->acceptVisitor(*this);
        mShaderOutput << ";";
    }
    mShaderOutput << "}";
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
    mShaderOutput << node.toString() << ".0f";
}

void CBasedFrontend::visitNode(const OperatorCallNode& node) {
    INodeVisitor::visitNode(node);
    static AMap<AString, AString> mInternalFunctions = {
            {"vec2", "glm::vec2"},
            {"vec3", "glm::vec3"},
            {"vec4", "glm::vec4"},
            {"sin",  "glm::sin"},
            {"cos",  "glm::cos"},
            {"tan",  "glm::tan"},
            {"atan", "glm::atan"},
    };

    if (auto i = mInternalFunctions.contains(node.getCallee())) {
        mShaderOutput << i->second << "(";
        bool first = true;
        for (const auto& arg: node.getArgs()) {
            if (first) {
                first = false;
            } else {
                mShaderOutput << ",";
            }
            arg->acceptVisitor(*this);
        }
        mShaderOutput << ")";
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
    mShaderOutput << ";";
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
    emitBinaryOperator("-", node);
}

void CBasedFrontend::visitNode(const BinaryPlusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("+", node);
}

void CBasedFrontend::visitNode(const UnaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "-";
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
}

void CBasedFrontend::visitNode(const FloatNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << "{}f"_format(node.getNumber());
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
                reportError(node, "output could be defined only for vsh");
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
            if (mInterDefined) {
                reportError(node, "inter is already defined");
            }
            mInterDefined = true;
            break;

        default:
            throw AException("internal compiler error");
    }
}


void CBasedFrontend::visitNode(const VariableReferenceNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << node.getVariableName();
}

AString CBasedFrontend::shaderCode() {
    return mShaderOutput.str();
}

void CBasedFrontend::writeCpp(const APath& destination) {
    auto namespaceName = "aui::sl_gen::{}"_format(destination.filenameWithoutExtension().replacedAll(".", "::"));
    const auto headerPath = destination.extensionChanged("h");
    // cpp
    {
        AFileOutputStream fos(destination);
        fos << "// This file is autogenerated by aui.toolbox. Please do not modify.\n";
        fos << "#include \"" << headerPath.filename() << "\"\n";
        fos << "using namespace " << namespaceName << ";\n";
        emitHelperFunctionsCpp(fos);
    }

    // header
    {

        AFileOutputStream fos(headerPath);
        fos << "// This file is autogenerated by aui.toolbox. Please do not modify.\n";
        fos << "#include <AUI/SL/SLShader.h>\n";
        fos << "#include <glm/glm.hpp>\n";
        fos << "namespace " << namespaceName << " {\n";
        emitHeaderDefinition(fos);
        fos << "}\n";
    }
}

void CBasedFrontend::emitHeaderDefinition(aui::no_escape<IOutputStream> os) const {
    *os << "_unique<SLShader> createShader();\n";
}

void CBasedFrontend::emitHelperFunctionsCpp(aui::no_escape<IOutputStream> os) const {

}
