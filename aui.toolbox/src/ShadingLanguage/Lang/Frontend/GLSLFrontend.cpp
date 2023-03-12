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

#include "CBasedFrontend.h"
#include "CppFrontend.h"
#include "AUI/Common/AMap.h"
#include "AUI/Logging/ALogger.h"
#include "GLSLFrontend.h"

AString GLSLFrontend::mapType(const AString& type) {
    const AMap<AString, AString> mapping = {
            {"vec2",   "vec2"},
            {"vec3",   "vec3"},
            {"vec4",   "vec4"},
            {"mat2",   "mat2"},
            {"mat3",   "mat3"},
            {"mat4",   "mat4"},
            {"float",  "float"},
            {"double", "double"},
            {"int",    "int"},
    };
    if (auto c = mapping.contains(type)) {
        return c->second;
    }
    throw AException("unsupported type: {}"_format(type));
}

void GLSLFrontend::visitNode(const IndexedAttributeDeclarationNode& node) {
    CBasedFrontend::visitNode(node);
    AString keyword;
    switch (node.type()) {
        case KeywordToken::INPUT:
            if (isVertex()) {
                keyword = "attribute";
                break;
            }
            // fallthrough
        case KeywordToken::OUTPUT:
            keyword = "varying";
            break;
        case KeywordToken::UNIFORM:
            keyword = "uniform";
            break;

        default:
            assert(0);
    }

    const auto decls = node.fields().toVector().sort([](const auto& l, const auto& r) {
        return std::get<int>(l) < std::get<int>(r);
    });
    for (const auto& [index, declaration]: decls) {
        mOutput << "/* " << AString::number(index) << " */ " << keyword << " ";
        static_cast<INodeVisitor*>(this)->visitNode(*declaration);
    }
}

void GLSLFrontend::parseShader(const _<AST>& ast) {
    mOutput << "#version 120\n";
    CBasedFrontend::parseShader(ast);
}

void GLSLFrontend::emitBeforeEntryCode() {
    mOutput << "void main(){";
}

void GLSLFrontend::emitAfterEntryCode() {
    mOutput << "}";
}

void GLSLFrontend::visitNode(const MemberAccessOperatorNode& node) {
    if (auto var = _cast<VariableReferenceNode>(node.getLeft())) {
        if (var->getVariableName() == "input" ||
            var->getVariableName() == "output" ||
            var->getVariableName() == "uniform") {
            // skip input, output, uniform accessors
            node.getRight()->acceptVisitor(*this);
            return;
        }
    }
    CBasedFrontend::visitNode(node);
}

void GLSLFrontend::visitNode(const VariableReferenceNode& node) {
    if (node.getVariableName() == "sl_position") {
        mOutput << "gl_Position";
    } else {
        CBasedFrontend::visitNode(node);
    }
}

