//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AVector.h>
#include "AnyToken.h"
#include <AUI/Reflect/AClass.h>
#include <ShadingLanguage/Lang/AST/VariableDeclarationNode.h>
#include <ShadingLanguage/Lang/AST/ExpressionNode.h>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/AException.h>

AUI_ENUM_INT(RequiredPriority) {
    ANY,

    COMPARE_OPERATORS, // ==, !=, <=, >=, <, >

    LOW_PRIORITY, // for +, -, |, ||
    HIGH_PRIORITY, // for *, /, &, &&

    /**
     * Parse expressions only with highest priority (unary operators, member access, functions call, etc...)
     */
    UNARY,

};

class Parser {
private:

    unsigned mErrorCount = 0;
    AVector<AnyToken> mTokens;
    AVector<AnyToken>::iterator mIterator = mTokens.begin();

    void skipUntilSemicolonOrNewLine();

    bool parseUsing();
    void reportUnexpectedErrorAndSkip(const AString& string);
    void reportUnexpectedEof();
    void reportError(const AString& message);
    AString getTokenName();

    template<typename T>
    T& expect() {
        if (std::holds_alternative<T>(*mIterator)) {
            return std::get<T>(*mIterator);
        }
        reportUnexpectedErrorAndSkip("expected "_as + AClass<T>::name());
        throw ::AException();
    }


    _<ExpressionNode> parseMemberAccess();
    _<ExpressionNode> parseLambda();
    _<ExpressionNode> parseIdentifier();
    _<ExpressionNode> parseTernary(const _<ExpressionNode>& condition);
    AVector<_<INode>> parseConstructorInitializerList();
    _<INode> parseStructClassDefinition() ;

    /**
     * Parses modifiers typename variablename
     * where modifiers = none or const or static
     * @return
     */
    _<VariableDeclarationNode> parseVariableDeclaration();

    /**
     * Parses (expression1, expression2, ...)
     * @return
     */
    AVector<_<ExpressionNode>> parseCallArgs();

    AVector<_<ExpressionNode>> parseCurlyBracketsArgs();

    /**
     * Parses (typename1 variablename1, typename2 variablename2, ...)
     */
    AVector<_<VariableDeclarationNode>> parseFunctionDeclarationArgs();

    /**
     * Parses { command1; command2; ... }
     */
    AVector<_<INode>> parseCodeBlock();


    /**
     * Parses SomeType
     * Parses SomeType::ChildType
     */
    AString parseTypename();

public:
    Parser(const AVector<AnyToken>& tokens) : mTokens(tokens) {}

    AVector<_<INode>> parse();
    _<ExpressionNode> parseExpression(RequiredPriority requiredPriority = RequiredPriority::ANY);

    unsigned getCurrentLineNumber();
};


