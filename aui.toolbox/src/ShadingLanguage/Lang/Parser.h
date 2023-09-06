//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AVector.h>
#include "AnyToken.h"
#include "ShadingLanguage/Lang/AST/IndexedAttributesDeclarationNode.h"
#include "ShadingLanguage/Lang/AST/NonIndexedAttributesDeclarationNode.h"
#include "AUI/Util/ABitField.h"
#include "ShadingLanguage/Lang/AST/AST.h"
#include <AUI/Reflect/AClass.h>
#include <ShadingLanguage/Lang/AST/VariableDeclarationNode.h>
#include <ShadingLanguage/Lang/AST/ExpressionNode.h>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Common/AException.h>


class Parser {
public:
    Parser(const AVector<AnyToken>& tokens, APath fileDir) : mTokens(tokens), mFileDir(std::move(fileDir)) {}

    _<AST> parseShader();
    _<ExpressionNode> parseExpression();

    unsigned getCurrentLineNumber();

    _<INode> parseEntry();

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
     * Parses (import identifier)
     */
    _<INode> parseImportStatement();

    /**
     * Parses { command1; command2; ... }
     */
    AVector<_<INode>> parseCodeBlock();

private:
    unsigned mErrorCount = 0;
    AVector<AnyToken> mTokens;
    AVector<AnyToken>::iterator mIterator = mTokens.begin();
    APath mFileDir;

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

    void nextTokenAndCheckEof();

    IndexedAttributesDeclarationNode::Fields parseIndexedAttributes();
    NonIndexedAttributesDeclarationNode::Fields parseNonIndexedAttributes();


    /**
     * Parses SomeType
     * Parses SomeType::ChildType
     */
    AString parseTypename();
};


