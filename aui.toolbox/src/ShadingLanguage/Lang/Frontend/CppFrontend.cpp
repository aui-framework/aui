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
// Created by Alex2772 on 3/5/2023.
//

#include <range/v3/view.hpp>
#include "CppFrontend.h"
#include "AUI/Common/AMap.h"
#include "AUI/Logging/ALogger.h"
#include "CBasedFrontend.h"

const AMap<AString, AString>& CppFrontend::internalFunctions() {
    static AMap<AString, AString> internalFunctions = {
            {"vec2", "glm::vec2"},
            {"vec3", "glm::vec3"},
            {"vec4", "glm::vec4"},
            {"mat3", "glm::mat3"},
            {"mat4", "glm::mat4"},
            {"sin",  "glm::sin"},
            {"sign", "glm::sign"},
            {"abs",  "glm::abs"},
            {"cos",  "glm::cos"},
            {"tan",  "glm::tan"},
            {"atan", "glm::atan"},
            {"sqrt", "glm::sqrt"},
            {"clamp", "glm::clamp"},
            {"step",  "glm::step"},
            {"mix",   "glm::mix"},
            {"dot",   "glm::dot"},
            {"mod",   "glm::mod"},
    };
    return internalFunctions;
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
            {"2D",     "Texture2D"},
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
    mHeaderNonIndexedAttributesDeclarations << node;
}

void CppFrontend::visitNode(const ArrayAccessOperatorNode& node) {
    INodeVisitor::visitNode(node);
    node.getLeft()->acceptVisitor(*this);
    mShaderOutput << "[";
    node.getRight()->acceptVisitor(*this);
    mShaderOutput << "]";
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

        case KeywordToken::TEXTURE:
            mHeaderOutput << "Texture";
            break;

        default:
            AUI_ASSERT(0);
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
        mShaderOutput << "\nShader::Inter Shader::entry(const Shader::Input& input, const Shader::Uniform& uniform){";
    } else {
        mShaderOutput << "\nShader::Output Shader::entry(const Shader::Inter& inter, const Shader::Uniform& uniform){";
    }
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

void CppFrontend::emitHeaderDefinition(aui::no_escape<IOutputStream> os) {
    for (const auto& type : {KeywordToken::Type::UNIFORM, KeywordToken::Type::INTER}) {
        auto range = mHeaderNonIndexedAttributesDeclarations
                     | ranges::views::filter([&](const auto& n) { return n.type() == type; });
        if (range.empty()) {
            continue;
        }

        mHeaderOutput << "struct ";

        emitAttributeKeyword(type);

        mHeaderOutput << "{";
        if (shaderType() == ShaderType::VERTEX && type == KeywordToken::INTER) {
            mHeaderOutput << "glm::vec4 __vertexOutput;";
        }

        ASet<AString> definedFields;
        for (const auto& node : range) {
            for (const auto& n: node.fields()) {
                if (node.type() == KeywordToken::TEXTURE) {
                    const ASet<AString> ALLOWED_TYPE_NAMES = {"2D"};
                    if (!ALLOWED_TYPE_NAMES.contains(n->typeName())) {
                        reportError(*n,
                                    "'{}' type is not allowed in texture {{ ... }} declaration"_format(n->typeName()));
                    }
                }
                if (definedFields.contains(n->variableName())) {
                    continue;
                }
                definedFields << n->variableName();
                emitAttributeDeclarationField(n);
            }
        }
        mHeaderOutput << "};";
    }
    *os << "struct Shader {";
    *os << mHeaderOutput.str();
    if (shaderType() == ShaderType::VERTEX) {
        *os << "\nstatic Inter entry(const Input& input, const Uniform& uniform);";
    } else {
        *os << "\nstatic Output entry(const Inter& inter, const Uniform& uniform);";
    }
    *os << "};\n";
}

void CppFrontend::emitCppCreateShader(aui::no_escape<IOutputStream> os) {
    CBasedFrontend::emitCppCreateShader(os);
    *os << mShaderOutput.str();
    *os << "}\n";
}

static bool isSwizzling(const AString& v) {
    if (v.length() < 2 || v.length() > 4) {
        return false;
    }

    return std::all_of(v.begin(), v.end(), [](auto c) {
        switch (c) {
            case 'x':
            case 'y':
            case 'z':
            case 'w':
            case 'r':
            case 'g':
            case 'b':
            case 'a':
                return true;

            default:
                return false;
        }
    });

}

void CppFrontend::visitNode(const MemberAccessOperatorNode& node) {
    CBasedFrontend::visitNode(node);

    if (auto c = _cast<VariableReferenceNode>(node.getRight())) {
        if (isSwizzling(c->getVariableName())) {
            mShaderOutput << "()";
        }
    }
}

void CppFrontend::visitNode(const FunctionDeclarationNode& node) {
    if (node.getName() != "entry") {
        mShaderOutput << "inline ";
    }
    CBasedFrontend::visitNode(node);
}

void CppFrontend::emitFunctionDeclArguments(const FunctionDeclarationNode& node, bool first) {
    mShaderOutput << "const Shader::Uniform& uniform";
    CBasedFrontend::emitFunctionDeclArguments(node, false);
}

void CppFrontend::emitFunctionCallArguments(const BuiltinOrDeclaredFunction& function, const AVector<_<ExpressionNode>>& args, bool first) {
    if (std::holds_alternative<const FunctionDeclarationNode*>(function)) {
        mShaderOutput << "uniform";
        CBasedFrontend::emitFunctionCallArguments(function, args, false);
    } else {
        CBasedFrontend::emitFunctionCallArguments(function, args, first);
    }
}
void CppFrontend::visitNode(const FloatNode& node) {
    INodeVisitor::visitNode(node);
    auto str = "{:0.9}f"_format(node.getNumber());
    if (!str.contains('.')) {
        str.pop_back();
        str += ".f";
    }
    mShaderOutput << str;
}

void CppFrontend::visitNode(const IntegerNode& node) {
    INodeVisitor::visitNode(node);
    mShaderOutput << node.toString() << ".0f";
}
