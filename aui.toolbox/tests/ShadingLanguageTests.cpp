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

#include <gmock/gmock.h>
#include "AUI/Common/AString.h"
#include "AUI/IO/AStringStream.h"
#include "ShadingLanguage/Lang/Frontend/CppFrontend.h"
#include "ShadingLanguage/Lang/SL.h"
#include "ShadingLanguage/Lang/Frontend/GLSLFrontend.h"
#include "ShadingLanguage/Lang/Lexer.h"
#include "ShadingLanguage/Lang/Parser.h"


class ShadingLanguage : public ::testing::Test {
protected:

    AString codeBlockToCpp(const AString& input) {
        CppFrontend compiler;
        compiler.visitCodeBlock(aui::sl::parseCodeBlock(input));
        return compiler.shaderCode();
    }

    template<aui::derived_from<IFrontend> T>
    AString vertexTo(const AString& input) {
        T compiler;
        compiler.setShaderType(ShaderType::VERTEX);
        compiler.parseShader(aui::sl::parseCode(_new<AStringStream>(input)));
        AStringStream ss;
        compiler.writeCppCpp("generic.h", ss);
        auto n = ss.str().find("{");
        if (n == std::string::npos) {
            return ss.str();
        }
        return ss.str().substr(n);
    }
    template<aui::derived_from<IFrontend> T>
    AString fragmentTo(const AString& input) {
        T compiler;
        compiler.setShaderType(ShaderType::FRAGMENT);
        compiler.parseShader(aui::sl::parseCode(_new<AStringStream>(input)));
        AStringStream ss;
        compiler.writeCppCpp("generic.h", ss);
        auto n = ss.str().find("{");
        if (n == std::string::npos) {
            return ss.str();
        }
        return ss.str().substr(n);
    }
};

TEST_F(ShadingLanguage, Simplest) {
    EXPECT_EQ(codeBlockToCpp("vec4 kek = vec4(1, 2, 3, 4)"), "glm::vec4 kek = glm::vec4(1.0f,2.0f,3.0f,4.0f);");
}
TEST_F(ShadingLanguage, TwoLines) {
    EXPECT_EQ(codeBlockToCpp("vec4 kek = vec4(1, 2, 3, 4)\nvec4 kek2 = vec4(4, 5, 6, 7)"), "glm::vec4 kek = glm::vec4(1.0f,2.0f,3.0f,4.0f);glm::vec4 kek2 = glm::vec4(4.0f,5.0f,6.0f,7.0f);");
}
TEST_F(ShadingLanguage, BasicShader) {
    const auto code = R"(
input {
  [0] vec4 pos
}

entry {
  sl_position = input.pos
}
)";
    EXPECT_STREQ(vertexTo<CppFrontend>(code).toStdString().c_str(), "{Shader::Inter inter;inter.__vertexOutput=input.pos;return inter;}\n");
    EXPECT_STREQ(vertexTo<GLSLFrontend>(code).toStdString().c_str(), "{ return R\"(#version 120\n/* 0 */ attribute vec4 SL_input_pos;void main(){gl_Position=SL_input_pos;} )\";}void ::Shader::setup() {}");
}

TEST_F(ShadingLanguage, Texture) {
    const auto code = R"(
output {
  [0] vec4 albedo
}
texture {
  2D albedo
}

entry {
  output.albedo = texture.albedo[vec2(0, 0)]
}
)";
    EXPECT_STREQ(fragmentTo<CppFrontend>(code).toStdString().c_str(), "{Shader::Inter inter;inter.__vertexOutput=input.pos;return inter;}\n");
    EXPECT_STREQ(fragmentTo<GLSLFrontend>(code).toStdString().c_str(), "{ return R\"(#version 120\n/* 0 */ attribute vec4 SL_input_pos;void main(){gl_Position=SL_input_pos;} )\";}void ::Shader::setup() {}");
}

TEST_F(ShadingLanguage, Math1) {
    Lexer l(_new<AStringStream>("x = 1 + 2 * 3\n"));
    Parser p(l.performLexAnalysis());
    auto expr = p.parseExpression();

    auto eq = _cast<AssignmentOperatorNode>(expr);
    ASSERT_TRUE(eq);

    auto sum = _cast<BinaryPlusOperatorNode>(eq->getRight());
    ASSERT_TRUE(sum);

    auto one = _cast<IntegerNode>(sum->getLeft());
    ASSERT_TRUE(one);
    ASSERT_EQ(one->getNumber(), 1);

    auto mul = _cast<BinaryAsteriskOperatorNode>(sum->getRight());
    ASSERT_TRUE(mul);

    auto two = _cast<IntegerNode>(mul->getLeft());
    ASSERT_TRUE(two);
    ASSERT_EQ(two->getNumber(), 2);

    auto three = _cast<IntegerNode>(mul->getRight());
    ASSERT_TRUE(three);
    ASSERT_EQ(three->getNumber(), 3);
}

TEST_F(ShadingLanguage, Math2) {
    Lexer l(_new<AStringStream>("x = 1 * 2 + 3\n"));
    Parser p(l.performLexAnalysis());
    auto expr = p.parseExpression();

    auto eq = _cast<AssignmentOperatorNode>(expr);
    ASSERT_TRUE(eq);

    auto sum = _cast<BinaryPlusOperatorNode>(eq->getRight());
    ASSERT_TRUE(sum);

    auto mul = _cast<BinaryAsteriskOperatorNode>(sum->getLeft());
    ASSERT_TRUE(mul);

    auto one = _cast<IntegerNode>(mul->getLeft());
    ASSERT_TRUE(one);
    ASSERT_EQ(one->getNumber(), 1);

    auto two = _cast<IntegerNode>(mul->getRight());
    ASSERT_TRUE(two);
    ASSERT_EQ(two->getNumber(), 2);

    auto three = _cast<IntegerNode>(sum->getRight());
    ASSERT_TRUE(three);
    ASSERT_EQ(three->getNumber(), 3);

}

TEST_F(ShadingLanguage, Math3) {
    //                                b1   b3   b5  b6   b7    b4   b2
    Lexer l(_new<AStringStream>("x = 1 + (2 + (3 + 4 * (5 * 6)) * 7) * 8\n"));
    Parser p(l.performLexAnalysis());
    auto expr = p.parseExpression();

    auto eq = _cast<AssignmentOperatorNode>(expr);
    ASSERT_TRUE(eq);

    auto b1 = _cast<BinaryPlusOperatorNode>(eq->getRight());
    ASSERT_TRUE(b1);

    {
        auto one = _cast<IntegerNode>(b1->getLeft());
        ASSERT_TRUE(one);
        ASSERT_EQ(one->getNumber(), 1);
    }
    {
        auto b2 = _cast<BinaryAsteriskOperatorNode>(b1->getRight());
        ASSERT_TRUE(b2);

        {
            auto eight = _cast<IntegerNode>(b2->getRight());
            ASSERT_TRUE(eight);
            ASSERT_EQ(eight->getNumber(), 8);
        }
        {
            auto b3 = _cast<BinaryPlusOperatorNode>(b2->getLeft());
            ASSERT_TRUE(b3);

            {
                auto i = _cast<IntegerNode>(b3->getLeft());
                ASSERT_TRUE(i);
                ASSERT_EQ(i->getNumber(), 2);
            }
            {
                auto b4 = _cast<BinaryAsteriskOperatorNode>(b3->getRight());
                ASSERT_TRUE(b4);
                {
                    auto i = _cast<IntegerNode>(b4->getRight());
                    ASSERT_TRUE(i);
                    ASSERT_EQ(i->getNumber(), 7);
                }
            }
        }
    }
}

TEST_F(ShadingLanguage, MemberAccess) {
    Lexer l(_new<AStringStream>("output.albedo.a = output.albedo.a + 1\n"));
    Parser p(l.performLexAnalysis());
    auto expr = p.parseExpression();

    auto eq = _cast<AssignmentOperatorNode>(expr);
    ASSERT_TRUE(eq);

    auto b = _cast<BinaryPlusOperatorNode>(eq->getRight());
    ASSERT_TRUE(b);

    auto i = _cast<IntegerNode>(b->getRight());
    ASSERT_TRUE(i);
    ASSERT_EQ(i->getNumber(), 1);
}