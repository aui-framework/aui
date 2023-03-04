//
// Created by alex2 on 6/12/2021.
//

#pragma once

#include <variant>
#include <ShadingLanguage/Lang/Token/IToken.h>
#include <ShadingLanguage/Lang/Token/IdentifierToken.h>
#include <ShadingLanguage/Lang/Token/IntegerToken.h>
#include <ShadingLanguage/Lang/Token/ColonToken.h>
#include <ShadingLanguage/Lang/Token/BracketsToken.h>
#include <ShadingLanguage/Lang/Token/StringToken.h>
#include <ShadingLanguage/Lang/Token/CommaToken.h>
#include <ShadingLanguage/Lang/Token/LShiftToken.h>
#include <ShadingLanguage/Lang/Token/RShiftToken.h>
#include <ShadingLanguage/Lang/Token/PlusToken.h>
#include <ShadingLanguage/Lang/Token/MinusToken.h>
#include <ShadingLanguage/Lang/Token/AsteriskToken.h>
#include <ShadingLanguage/Lang/Token/DivideToken.h>
#include <ShadingLanguage/Lang/Token/PointerFieldAccessToken.h>
#include <ShadingLanguage/Lang/Token/FieldAccessToken.h>
#include <ShadingLanguage/Lang/Token/LogicalNotToken.h>
#include <ShadingLanguage/Lang/Token/AmpersandToken.h>
#include <ShadingLanguage/Lang/Token/EqualToken.h>
#include <ShadingLanguage/Lang/Token/TernaryToken.h>
#include <ShadingLanguage/Lang/Token/PreprocessorDirectiveToken.h>
#include <ShadingLanguage/Lang/Token/SemicolonToken.h>
#include <ShadingLanguage/Lang/Token/DoubleColonToken.h>
#include <ShadingLanguage/Lang/Token/KeywordToken.h>
#include <ShadingLanguage/Lang/Token/BitwiseOrToken.h>
#include <ShadingLanguage/Lang/Token/LogicalOrToken.h>
#include <ShadingLanguage/Lang/Token/LogicalAndToken.h>
#include <ShadingLanguage/Lang/Token/ModToken.h>
#include <ShadingLanguage/Lang/Token/NotEqualToken.h>
#include <ShadingLanguage/Lang/Token/DoubleEqualToken.h>
#include <ShadingLanguage/Lang/Token/FloatToken.h>
#include <ShadingLanguage/Lang/Token/NewLineToken.h>

using AnyToken = std::variant<IdentifierToken,
                              IntegerToken,
                              ColonToken,
                              LParToken,
                              RParToken,
                              LCurlyBracketToken,
                              RCurlyBracketToken,
                              LSquareBracketToken,
                              RSquareBracketToken,
                              LAngleBracketToken,
                              RAngleBracketToken,
                              StringToken,
                              CommaToken,
                              LShiftToken,
                              RShiftToken,
                              PlusToken,
                              MinusToken,
                              AsteriskToken,
                              DivideToken,
                              PointerFieldAccessToken,
                              FieldAccessToken,
                              LogicalNotToken,
                              AmpersandToken,
                              EqualToken,
                              TernaryToken,
                              PreprocessorDirectiveToken,
                              SemicolonToken,
                              DoubleColonToken,
                              KeywordToken,
                              BitwiseOrToken,
                              LogicalAndToken,
                              LogicalOrToken,
                              ModToken,
                              DoubleEqualToken,
                              NotEqualToken,
                              FloatToken,
                              NewLineToken>;