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
#include "CBasedFrontend.h"

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

void CppFrontend::visitNode(const IndexedAttributeDeclarationNode& node) {
    CBasedFrontend::visitNode(node);
    mShaderOutput << "struct ";

    switch (node.type()) {
        case KeywordToken::INPUT:
            mShaderOutput << "Input";
            break;

        case KeywordToken::OUTPUT:
            mShaderOutput << "Output";
            break;

        case KeywordToken::UNIFORM:
            mShaderOutput << "Uniform";
            break;

        default:
            assert(0);
    }

    mShaderOutput << "{";
    if (isVertex() && node.type() == KeywordToken::OUTPUT) {
        mShaderOutput << "glm::vec4 __vertexOutput;";
    }
    const auto decls = node.fields().toVector().sort([](const auto& l, const auto& r) {
        return std::get<int>(l) < std::get<int>(r);
    });
    for (const auto& [index, declaration]: decls) {
        mShaderOutput << "/* " << AString::number(index) << " */";
        static_cast<INodeVisitor*>(this)->visitNode(*declaration);
    }
    mShaderOutput << "};";
}

void CppFrontend::emitAfterEntryCode() { mShaderOutput << "return output;}"; }

void CppFrontend::emitBeforeEntryCode() { mShaderOutput << "Output entry(Input input){Output output;"; }

void CppFrontend::visitNode(const VariableReferenceNode& node) {

    if (node.getVariableName() == "sl_position") {
        mShaderOutput << "output.__vertexOutput";
    } else {
        CBasedFrontend::visitNode(node);
    }
}