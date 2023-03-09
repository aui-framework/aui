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

#include "CppCompiler.h"
#include "AUI/Common/AMap.h"

void CppCompiler::visitNode(const EqualsOperatorNode& node) {

}

void CppCompiler::visitNode(const NotEqualsOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ConstructorDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const RShiftOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const MemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const StaticMemberAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const AssignmentOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ExplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ImplicitInitializerListCtorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const FunctionDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LambdaNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const MethodDeclarationNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const NullptrNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const IntegerNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << node.toString() << ".0f";
}

void CppCompiler::visitNode(const OperatorCallNode& node) {
    INodeVisitor::visitNode(node);
    static AMap<AString, AString> mInternalFunctions = {
        {"vec2", "glm::vec2"},
        {"vec3", "glm::vec3"},
        {"vec4", "glm::vec4"},
        {"sin", "glm::sin"},
        {"cos", "glm::cos"},
        {"tan", "glm::tan"},
        {"atan", "glm::atan"},
    };

    if (auto i = mInternalFunctions.contains(node.getCallee())) {
        mOutput << i->second << "(";
        bool first = true;
        for (const auto& arg : node.getArgs()) {
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

void CppCompiler::visitNode(const OperatorLiteralNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const StringNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const TemplateOperatorCallNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const TernaryOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ThisNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ReturnOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LogicalNotOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const PointerDereferenceOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const PointerCreationOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const VariableDeclarationNode& node) {
    INodeVisitor::visitNode(node);
    mOutput << mapType(node.typeName()) << " " << node.variableName();
    if (const auto& init = node.initializer()) {
        mOutput << " = ";
        init->acceptVisitor(*this);
    }
    mOutput << ";";
}

void CppCompiler::visitNode(const VariableReferenceNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LogicalAndOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LogicalOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const BitwiseOrOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ModOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const BinaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const BinaryPlusOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const UnaryMinusOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const IfOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const StructClassDefinition& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const GreaterOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const LessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const BoolNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const TemplateOperatorTypenameNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const ArrayAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
}

void CppCompiler::visitNode(const FloatNode& node) {
    INodeVisitor::visitNode(node);
}

AString CppCompiler::mapType(const AString& type) {
    const AMap<AString, AString> mapping = {
        { "vec2", "glm::vec2" },
        { "vec3", "glm::vec3" },
        { "vec4", "glm::vec4" },
        { "mat2", "glm::mat2" },
        { "mat3", "glm::mat3" },
        { "mat4", "glm::mat4" },
        { "float", "float" },
        { "double", "double" },
        { "int", "int" },
    };
    if (auto c = mapping.contains(type)) {
        return c->second;
    }
    throw AException("unsupported type: {}"_format(type));
}

void CppCompiler::vertex(const _<AST>& ast) {

}
