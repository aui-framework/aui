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
// Created by Alex2772 on 3/5/2023.
//

#include "CppFrontend.h"
#include "AUI/Common/AMap.h"
#include "AUI/Logging/ALogger.h"

void CppFrontend::visitNode(const EqualsOperatorNode& node) {
    emitBinaryOperator("==", node);
}

void CppFrontend::visitNode(const NotEqualsOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("!=", node);
}

void CppFrontend::visitNode(const ConstructorDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const LShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const RShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const MemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator(".", node);
}

void CppFrontend::visitNode(const StaticMemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const AssignmentOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("=", node);
}

void CppFrontend::visitNode(const ExplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const ImplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const FunctionDeclarationNode& node) {
    INodeVisitor::visitNode(node);

    if (node.getName() == "entry") {
        // entry function
        mEntry = node.getCode();
        return;
    }
    mOutput << mapType(node.getReturnType()) << " " << node.getName() << "(";
    bool first = true;
    for (const auto& v: node.getArgs()) {
        if (first) {
            first = false;
        } else {
            mOutput << ",";
        }
        visitNode(*v);
    }
    mOutput << "){";
    for (const auto& v: node.getCode()) {
        v->acceptVisitor(*this);
        mOutput << ";";
    }
    mOutput << "}";
}

void CppFrontend::visitNode(const LambdaNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const MethodDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const NullptrNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const IntegerNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << node.toString() << ".0f";
}

void CppFrontend::visitNode(const OperatorCallNode& node) {
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
        mOutput << i->second << "(";
        bool first = true;
        for (const auto& arg: node.getArgs()) {
            if (first) {
                first = false;
            } else {
                mOutput << ",";
            }
            arg->acceptVisitor(*this);
        }
        mOutput << ")";
    }
}

void CppFrontend::visitNode(const OperatorLiteralNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const StringNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const TemplateOperatorCallNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const TernaryOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const ThisNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const ReturnOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const LogicalNotOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const PointerDereferenceOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const PointerCreationOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const VariableDeclarationNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << mapType(node.typeName()) << " " << node.variableName();
    if (const auto& init = node.initializer()) {
        mOutput << " = ";
        init->acceptVisitor(*this);
    }
    mOutput << ";";
}

void CppFrontend::visitNode(const VariableReferenceNode& node) {
    INodeVisitor::visitNode(node);

    if (node.getVariableName() == "sl_position") {
        mOutput << "output.__vertexOutput";
    } else {
        mOutput << node.getVariableName();
    }
}

void CppFrontend::visitNode(const LogicalAndOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("&&", node);
}

void CppFrontend::visitNode(const LogicalOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("||", node);
}

void CppFrontend::visitNode(const BitwiseOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("|", node);
}

void CppFrontend::visitNode(const ModOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("%", node);
}

void CppFrontend::visitNode(const BinaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("-", node);
}

void CppFrontend::visitNode(const BinaryPlusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("+", node);
}

void CppFrontend::visitNode(const UnaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << "-";
}

void CppFrontend::visitNode(const IfOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const StructClassDefinition& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const GreaterOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator(">", node);
}

void CppFrontend::visitNode(const LessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    emitBinaryOperator("<", node);
}

void CppFrontend::visitNode(const BoolNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << AString::number(node.getValue());
}

void CppFrontend::visitNode(const TemplateOperatorTypenameNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const ArrayAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppFrontend::visitNode(const FloatNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << "{}f"_format(node.getNumber());
}

AString CppFrontend::mapType(const AString& type) {
    const AMap<AString, AString> mapping = {
            {"vec2",   "glm::vec2"},
            {"vec3",   "glm::vec3"},
            {"vec4",   "glm::vec4"},
            {"mat2",   "glm::mat2"},
            {"mat3",   "glm::mat3"},
            {"mat4",   "glm::mat4"},
            {"float",  "float"},
            {"double", "double"},
            {"int",    "int"},
    };
    if (auto c = mapping.contains(type)) {
        return c->second;
    }
    throw AException("unsupported type: {}"_format(type));
}

void CppFrontend::parseShader(const _<AST>& ast) {
    ast->visit(*this);

    if (!mInputDefined) {
        throw AException("no input defined in this compilation unit");
    }

    if (!mEntry) {
        throw AException("no entry point in this compilation unit");
    }

    if (!mOutputDefined) {
        visitNode(IndexedAttributeDeclarationNode{KeywordToken::OUTPUT, {}});
    }

    mOutput << "Output entry(Input input){Output output;";
    for (const auto& c: *mEntry) {
        c->acceptVisitor(*this);
        mOutput << ";";
    }
    mOutput;
}

void CppFrontend::visitNode(const IndexedAttributeDeclarationNode& node) {
    mOutput << "struct ";
    switch (node.type()) {
        case KeywordToken::INPUT:
            mOutput << "Input";
            if (mInputDefined) {
                reportError(node, "input is already defined");
            }
            mInputDefined = true;
            break;

        case KeywordToken::OUTPUT:
            mOutput << "Output";
            if (mOutputDefined) {
                reportError(node, "output is already defined");
            }
            mOutputDefined = true;
            break;

        case KeywordToken::UNIFORM:
            mOutput << "Uniform";
            if (mUniformDefined) {
                reportError(node, "uniform is already defined");
            }
            mUniformDefined = true;
            break;

        default:
            throw AException("internal compiler error");
    }

    mOutput << "{";
    if (isVertex() && node.type() == KeywordToken::OUTPUT) {
        mOutput << "glm::vec4 __vertexOutput;";
    }
    const auto decls = node.fields().toVector().sort([](const auto& l, const auto& r) {
        return std::get<int>(l) < std::get<int>(r);
    });
    for (const auto& [index, declaration]: decls) {
        mOutput << "/* " << AString::number(index) << " */";
        visitNode(*declaration);
    }
    mOutput << "};";
}

bool CppFrontend::isVertex() {
    return true;
}

void CppFrontend::emitBinaryOperator(const AString& symbol, const BinaryOperatorNode& binaryOperator) {
    binaryOperator.getLeft()->acceptVisitor(*this);
    mOutput << symbol;
    binaryOperator.getRight()->acceptVisitor(*this);
}

void CppFrontend::reportError(const INode& node, const AString& message) {
    ALogger::err("Frontend") << ":" << node.getLineNumber() << " " << message;
}
