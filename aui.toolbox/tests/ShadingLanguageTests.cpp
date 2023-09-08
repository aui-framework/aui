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
};

static AString toGlslExpression(const _<ExpressionNode>& expr) {
    GLSLFrontend glsl;
    expr->acceptVisitor(glsl);
    return glsl.shaderCode();
}

TEST_F(ShadingLanguage, Math1) {
    Lexer l(_new<AStringStream>(std::string("x = 1 + 2 * 3\n")));
    Parser p(l.performLexAnalysis(), "");
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
    Lexer l(_new<AStringStream>(std::string("x = 1 * 2 + 3\n")));
    Parser p(l.performLexAnalysis(), "");
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
    //                                             b1   b3   b5  b6   b7    b4   b2
    Lexer l(_new<AStringStream>(std::string("x = 1 + (2 + (3 + 4 * (5 * 6)) * 7) * 8\n")));
    Parser p(l.performLexAnalysis(), "");
    auto expr = p.parseExpression();
    EXPECT_STREQ("x=(1.0f+((2.0f+((3.0f+(4.0f*(5.0f*6.0f)))*7.0f))*8.0f))", toGlslExpression(expr).toStdString().c_str());

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


TEST_F(ShadingLanguage, Math4) {
    Lexer l(_new<AStringStream>(std::string("step(0.001, circle.x * circle.x + circle.y * circle.y)\n")));
    Parser p(l.performLexAnalysis(), "");
    auto expr = p.parseExpression();
    EXPECT_STREQ("step(0.001f,((circle.x*circle.x)+(circle.y*circle.y)))", toGlslExpression(expr).toStdString().c_str());
}
TEST_F(ShadingLanguage, Math5) {
    Lexer l(_new<AStringStream>(std::string("clamp(val1 + val2 + val3, 0, 1)\n")));
    Parser p(l.performLexAnalysis(), "");
    auto expr = p.parseExpression();
    EXPECT_STREQ("clamp(((val1+val2)+val3),0.0f,1.0f)", toGlslExpression(expr).toStdString().c_str());
}

TEST_F(ShadingLanguage, MemberAccess) {
    Lexer l(_new<AStringStream>(std::string("output.albedo.a = output.albedo.a + 1\n")));
    Parser p(l.performLexAnalysis(), "");
    auto expr = p.parseExpression();

    auto eq = _cast<AssignmentOperatorNode>(expr);
    ASSERT_TRUE(eq);

    auto b = _cast<BinaryPlusOperatorNode>(eq->getRight());
    ASSERT_TRUE(b);

    auto i = _cast<IntegerNode>(b->getRight());
    ASSERT_TRUE(i);
    ASSERT_EQ(i->getNumber(), 1);
}