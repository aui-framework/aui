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

//
// Created by Alex2772 on 3/12/2023.
//

#include "CBasedFrontend.h"
#include "CppFrontend.h"
#include "AUI/Common/AMap.h"
#include "AUI/Logging/ALogger.h"
#include "GLSLFrontend.h"

const AMap<AString, AString>& GLSLFrontend::internalFunctions() {
    static AMap<AString, AString> internalFunctions = {
            {"vec2", "vec2"},
            {"vec3", "vec3"},
            {"vec4", "vec4"},
            {"mat3", "mat3"},
            {"mat4", "mat4"},
            {"sin",  "sin"},
            {"cos",  "cos"},
            {"tan",  "tan"},
            {"atan", "atan"},
            {"sign", "sign"},
            {"abs",  "abs"},
            {"sqrt", "sqrt"},
            {"clamp","clamp"},
            {"step", "step"},
            {"mix",  "mix"},
            {"dot",  "dot"},
            {"mod",  "mod"},
    };
    return internalFunctions;
}

void GLSLFrontend::ensurePrecisionInserted() {
    if (!mPrecisionInsterted) {
        // shitty compilers like adreno very do want extension definitions before any other tokens for no actual reason
        mPrecisionInsterted = true;
        mShaderOutput <<
                     "precision highp float;\n"
                     "precision highp int;\n"
                     ;
    }
}

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
            {"2D",     "sampler2D"},
    };
    ensurePrecisionInserted();
    if (auto c = mapping.contains(type)) {
        return c->second;
    }
    throw AException("unsupported type: {}"_format(type));
}

void GLSLFrontend::visitNode(const IndexedAttributesDeclarationNode& node) {
    ensurePrecisionInserted();
    const auto byKey = [](const auto& l, const auto& r) {
        return std::get<0>(l) < std::get<0>(r);
    };
    CBasedFrontend::visitNode(node);
    switch (node.type()) {
        case KeywordToken::INPUT: {
            auto decls = node.fields().toVector().sort(byKey);
            for (const auto& [index, declaration]: decls) {
                mShaderOutput << "/* " << AString::number(index) << " */ attribute ";
                if (auto c = _cast<VariableDeclarationNode>(declaration)) {
                    mShaderOutput << c->typeName() << " SL_input_" << c->variableName() << ";";
                } else {
                    declaration->acceptVisitor(*this);
                }
            }
            mInputs = std::move(decls);
            break;
        }

        case KeywordToken::OUTPUT: {
            mOutputs = node.fields().toVector().sort(byKey).map([](const auto& t) { return std::get<1>(t); });
            break;
        }

        default:
            AUI_ASSERT(0);
    }

}
void GLSLFrontend::visitNode(const NonIndexedAttributesDeclarationNode& node) {
    ensurePrecisionInserted();
    CBasedFrontend::visitNode(node);
    AString keyword;
    AString prefix;
    switch (node.type()) {

        case KeywordToken::INTER:
            keyword = "varying";
            prefix = "inter";
            break;

        case KeywordToken::TEXTURE:
            keyword = "uniform";
            prefix = "texture";
            break;

        case KeywordToken::UNIFORM:
            prefix = keyword = "uniform";
            break;

        default:
            AUI_ASSERT(0);
    }

    for (const auto& declaration: node.fields()) {
        if (node.type() == KeywordToken::UNIFORM) {
            if (mDefinedUniforms.contains(declaration->variableName())) {
                continue;
            }
            mDefinedUniforms << declaration->variableName();
        }
        mShaderOutput << keyword << " ";
        mShaderOutput << mapType(declaration->typeName()) << " SL_" << prefix << "_" << declaration->variableName() << ";";
    }
}

void GLSLFrontend::parseShader(const _<AST>& ast) {
    mShaderOutput << "#version 100\n"
                     "#define glsl120\n"
                     ;

    CBasedFrontend::parseShader(ast);
}

void GLSLFrontend::emitBeforeEntryCode() {
    mShaderOutput << "void main(){";
}

void GLSLFrontend::emitAfterEntryCode() {
    mShaderOutput << "}";
}

void GLSLFrontend::visitNode(const MemberAccessOperatorNode& node) {
    if (auto var = _cast<VariableReferenceNode>(node.getLeft())) {
        if (var->getVariableName() == "input" ||
            var->getVariableName() == "uniform" ||
            var->getVariableName() == "inter") {
            if (auto c = _cast<VariableReferenceNode>(node.getRight())) {
                mShaderOutput << "SL_" << var->getVariableName() << "_" << c->getVariableName();
            } else {
                node.getRight()->acceptVisitor(*this);
            }
            return;
        }
        if (var->getVariableName() == "output") {
            // skip input, output, uniform accessors
            if (auto c = _cast<VariableReferenceNode>(node.getRight())) {
                if (auto it = mOutputs.findIf([&](const auto& v) {
                    return v->variableName() == c->getVariableName();
                })) {
                    if (&*mOutputs.begin() == it) {
                        mShaderOutput << "gl_FragColor";
                    } else {
                        mShaderOutput << "gl_FragData[" << AString::number(std::distance(&(*mOutputs.begin()), it)) << "]";
                    }
                }
            } else {
                node.getRight()->acceptVisitor(*this);
            }
            return;
        }
    }
    CBasedFrontend::visitNode(node);
}

void GLSLFrontend::visitNode(const VariableReferenceNode& node) {
    if (node.getVariableName() == "sl_position") {
        mShaderOutput << "gl_Position";
    } else {
        CBasedFrontend::visitNode(node);
    }
}


void GLSLFrontend::emitHeaderDefinition(aui::no_escape<IOutputStream> os) {
    *os << "struct Shader {"
           "  static const char* code();"
           "  static void setup(uint32_t program);"
           "};";
}

void GLSLFrontend::emitCppCreateShader(aui::no_escape<IOutputStream> os) {
    *os << "#include <AUI/GL/gl.h>\n";
    CBasedFrontend::emitCppCreateShader(os);

    *os << "const char* " << namespaceName() <<  "::Shader::code() { return R\"(" << mShaderOutput.str() << " )\";}"
           "void " << namespaceName() <<  "::Shader::setup(uint32_t program) {";
    for (const auto& [index, input]  : mInputs) {
      *os << "glBindAttribLocation(program, " << AString::number(index) << ", \"SL_input_" << input->variableName() << "\");";
    }
    *os << "}";
}
