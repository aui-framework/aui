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

void CppFrontend::visitNode(const IndexedAttributesDeclarationNode& node) {
    CBasedFrontend::visitNode(node);

    mHeaderOutput << "struct ";

    emitAttributeKeyword(node.type());

    mHeaderOutput << "{";
    if (shaderType() == ShaderType::VERTEX && node.type() == KeywordToken::OUTPUT) {
        mHeaderOutput << "glm::vec4 __vertexOutput;";
    }
    const auto decls = node.fields().toVector().sort([](const auto& l, const auto& r) {
        return std::get<int>(l) < std::get<int>(r);
    });
    for (const auto& [index, node]: decls) {
        mHeaderOutput << "/* " << AString::number(index) << " */";
        emitAttributeDeclarationField(node);
    }
    mHeaderOutput << "};";
}

void CppFrontend::visitNode(const NonIndexedAttributesDeclarationNode& node) {
    CBasedFrontend::visitNode(node);

    mHeaderOutput << "struct ";

    emitAttributeKeyword(node.type());

    mHeaderOutput << "{";
    if (shaderType() == ShaderType::VERTEX && node.type() == KeywordToken::INTER) {
        mHeaderOutput << "glm::vec4 __vertexOutput;";
    }
    for (const auto& node: node.fields()) {
        emitAttributeDeclarationField(node);
    }
    mHeaderOutput << "};";
}

void CppFrontend::emitAttributeDeclarationField(_<VariableDeclarationNode> node) {
    mHeaderOutput << mapType(node->typeName()) << " " << node->variableName();
    if (const auto& init = node->initializer()) {
        reportError(*init, "attribute declaration shouldn't have initializer");
    }
    mHeaderOutput << ";";
}

void CppFrontend::emitAttributeKeyword(KeywordToken::Type type) {
    switch (type) {
        case KeywordToken::INPUT:
            mHeaderOutput << "Input";
            break;

        case KeywordToken::OUTPUT:
            mHeaderOutput << "Output";
            break;

        case KeywordToken::UNIFORM:
            mHeaderOutput << "Uniform";
            break;

        case KeywordToken::INTER:
            mHeaderOutput << "Inter";
            break;

        default:
            assert(0);
    }
}

void CppFrontend::emitAfterEntryCode() {
    if (shaderType() == ShaderType::VERTEX) {
        mShaderOutput << "return inter;";
    } else {
        mShaderOutput << "return output;";
    }
}

void CppFrontend::emitBeforeEntryCode() {
    if (shaderType() == ShaderType::VERTEX) {
        mShaderOutput << "Shader::Inter inter;";
    } else {
        mShaderOutput << "Shader::Output output;";
    }
}

void CppFrontend::visitNode(const VariableReferenceNode& node) {

    if (node.getVariableName() == "sl_position") {
        mShaderOutput << "inter.__vertexOutput";
    } else {
        CBasedFrontend::visitNode(node);
    }
}

void CppFrontend::emitHeaderDefinition(aui::no_escape<IOutputStream> os) const {
    *os << "struct Shader: SLShader {";
    *os << mHeaderOutput.str();
    if (shaderType() == ShaderType::VERTEX) {
        *os << "\nstatic Inter entry(const Input& input, const Uniform& uniform);";
    } else {
        *os << "\nstatic Output entry(const Inter& inter, const Uniform& uniform);";
    }
    *os << "};\n";
}

void CppFrontend::emitHelperFunctionsCpp(aui::no_escape<IOutputStream> os) const {
    CBasedFrontend::emitHelperFunctionsCpp(os);
    if (shaderType() == ShaderType::VERTEX) {
        *os << "\nShader::Inter Shader::entry(const Shader::Input& input, const Shader::Uniform& uniform){";
    } else {
        *os << "\nShader::Output Shader::entry(const Shader::Inter& inter, const Shader::Uniform& uniform){";
    }
    *os << mShaderOutput.str();
    *os << "}\n";
}
