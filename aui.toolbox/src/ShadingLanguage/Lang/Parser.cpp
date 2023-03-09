//
// Created by alex2 on 6/12/2021.
//

#include <AUI/Logging/ALogger.h>
#include <ShadingLanguage/Lang/AST/ConstructorDeclarationNode.h>
#include <ShadingLanguage/Lang/AST/StringNode.h>
#include <ShadingLanguage/Lang/AST/IntegerNode.h>
#include <ShadingLanguage/Lang/AST/VariableReferenceNode.h>
#include <ShadingLanguage/Lang/AST/OperatorLiteralNode.h>
#include <ShadingLanguage/Lang/AST/OperatorCallNode.h>
#include <ShadingLanguage/Lang/AST/ExplicitInitializerListCtorNode.h>
#include <ShadingLanguage/Lang/AST/TemplateOperatorCallNode.h>
#include <ShadingLanguage/Lang/AST/BinaryOperatorNode.h>
#include <ShadingLanguage/Lang/AST/ThisNode.h>
#include <ShadingLanguage/Lang/AST/LambdaNode.h>
#include <ShadingLanguage/Lang/AST/NullptrNode.h>
#include <ShadingLanguage/Lang/AST/UnaryOperatorNode.h>
#include <ShadingLanguage/Lang/AST/ImplicitInitializerListCtorNode.h>
#include <ShadingLanguage/Lang/AST/TernaryOperatorNode.h>
#include <ShadingLanguage/Lang/AST/IfOperatorNode.h>
#include <ShadingLanguage/Lang/AST/StructClassDefinition.h>
#include <ShadingLanguage/Lang/AST/BoolNode.h>
#include <ShadingLanguage/Lang/AST/TemplateOperatorTypenameNode.h>
#include <ShadingLanguage/Lang/AST/FloatNode.h>
#include "Parser.h"

class Terminated {};
template<typename variant, typename type>
struct index_of {
private:
    template<size_t i, size_t... next>
    static constexpr size_t value_impl2(std::index_sequence<i, next...>) {
        if constexpr (std::is_same_v<std::variant_alternative_t<i, variant>, type>) {
            return i;
        } else {
            return value_impl2(std::index_sequence<next...>{});
        }
    }

    static constexpr size_t value_impl() {
        return value_impl2(std::make_index_sequence<std::variant_size_v<variant>>());
    }
public:
    static constexpr size_t value = value_impl();
};

template<typename type>
constexpr size_t got = index_of<AnyToken , type>::value;

AVector<_<INode>> Parser::parse() {
    AVector<_<INode>> nodes;
    try {
        for (; mIterator != mTokens.end(); [&] { if (mIterator != mTokens.end()) ++mIterator; }()) {
            try {
                // here we should find #includes and function definitions
                switch (mIterator->index()) {
                    case got<PreprocessorDirectiveToken>: {
                        break;
                    }
                    case got<SemicolonToken>: {
                        break;
                    }
                    case got<KeywordToken>: {
                        switch (std::get<KeywordToken>(*mIterator).getType()) {
                            case KeywordToken::USING:
                                parseUsing();
                                break;

                            case KeywordToken::CLASS:
                            case KeywordToken::STRUCT: { // class or struct definition
                                ++mIterator;
                                nodes << parseStructClassDefinition();
                                break;
                            }

                        }
                        break;
                    }
                    case got<IdentifierToken>: {
                        // variable or function definition
                        auto name1 = std::get<IdentifierToken>(*mIterator).value();
                        ++mIterator;
                        switch (mIterator->index()) {
                            case got<DoubleColonToken>:
                                // class constructor definition
                                ++mIterator;

                                switch (mIterator->index()) {
                                    case got<IdentifierToken>: {
                                        auto name2 = std::get<IdentifierToken>(*mIterator).value();
                                        ++mIterator;
                                        auto args = parseFunctionDeclarationArgs();
                                        auto initializerList = parseConstructorInitializerList();
                                        auto codeBlock = parseCodeBlock();
                                        nodes << _new<ConstructorDeclarationNode>(name1, name2, args, name1,
                                                                                  initializerList, codeBlock);
                                        //break;
                                        return nodes;
                                    }

                                    default:
                                        reportUnexpectedErrorAndSkip("expected name token for constructor definition");
                                }

                                break;

                            case got<IdentifierToken>:
                                // variable or function definition
                                // name1    name2
                                // result_t function_or_class_name
                                auto name2 = std::get<IdentifierToken>(*mIterator).value();
                                ++mIterator;

                                switch (mIterator->index()) {
                                    case got<SemicolonToken>:
                                        nodes << _new<VariableDeclarationNode>(false, false, name1, name2, 0, false);
                                        break;

                                    case got<EqualToken>:
                                        ++mIterator;
                                        nodes << _new<VariableDeclarationNode>(false, false, name1, name2, 0, false,
                                                                               parseExpression());
                                        break;

                                    case got<LParToken>:
                                        // function definition
                                        parseFunctionDeclarationArgs();
                                        break;

                                    case got<DoubleColonToken>:
                                        // class constructor definition

                                        break;
                                }

                                break;
                        }

                        break;
                    }
                    default:
                        reportError("unexpected \"" + getTokenName() + "\"");
                }
            } catch (const AException&) {}
        }
    } catch (Terminated) {}
    return nodes;
}

AVector<_<VariableDeclarationNode>> Parser::parseFunctionDeclarationArgs() {
    expect<LParToken>();
    AVector<_<VariableDeclarationNode>> result;
    ++mIterator;
    for (; mIterator != mTokens.end();) {
        switch (mIterator->index()) {
            case got<RParToken>:
                ++mIterator;
                return result;

            case got<CommaToken>:
                // next arg
                ++mIterator;
                break;

            case got<KeywordToken>:
            case got<IdentifierToken>:
                result << parseVariableDeclaration();
                break;

            default:
                reportUnexpectedErrorAndSkip("expected identifier, comma or right parenthesis");
                throw AException{};
        }
    }
    reportUnexpectedEof();
    throw AException{};
}
AVector<_<ExpressionNode>> Parser::parseCallArgs() {
    expect<LParToken>();
    AVector<_<ExpressionNode>> result;
    ++mIterator;
    for (; mIterator != mTokens.end();) {
        switch (mIterator->index()) {
            case got<RParToken>:
                ++mIterator;
                return result;

            case got<CommaToken>:
                // next arg
                ++mIterator;
                break;

            default:
                result << parseExpression();
        }
    }

    reportUnexpectedEof();
    throw AException{};
}
AVector<_<ExpressionNode>> Parser::parseCurlyBracketsArgs() {
    expect<LCurlyBracketToken>();
    AVector<_<ExpressionNode>> result;
    ++mIterator;
    for (; mIterator != mTokens.end();) {
        switch (mIterator->index()) {
            case got<RCurlyBracketToken>:
                ++mIterator;
                return result;

            case got<CommaToken>:
                // next arg
                ++mIterator;
                break;

            default:
                result << parseExpression();
        }
    }

    reportUnexpectedEof();
    throw AException{};
}

AVector<_<INode>> Parser::parseCodeBlock() {
    AVector<_<INode>> result;
    expect<LCurlyBracketToken>();
    ++mIterator;
    for (; mIterator != mTokens.end();) {
        switch (mIterator->index()) {
            case got<IdentifierToken>: {
                // variable declaration, variable assignment or function call
                result << parseExpression();
                break;
            }

            case got<SemicolonToken>:
                ++mIterator;
                break;

            case got<RCurlyBracketToken>:
                ++mIterator;
                return result;

            case got<KeywordToken>: {
                // TODO place for for, while, goto
                auto t = std::get<KeywordToken>(*mIterator).getType();
                ++mIterator;
                switch (t) {
                    case KeywordToken::RETURN:
                        result << _new<ReturnOperatorNode>(parseExpression());
                        break;

                    case KeywordToken::USING:
                        skipUntilSemicolonOrNewLine();
                        break;

                    case KeywordToken::IF: {
                        expect<LParToken>();
                        ++mIterator;
                        auto condition = parseExpression();
                        expect<RParToken>();
                        ++mIterator;
                        auto codeBlock = parseCodeBlock();
                        result << _new<IfOperatorNode>(condition, codeBlock);
                        break;
                    }

                    case KeywordToken::TRY: {
                        auto codeBlock = parseCodeBlock();
                        try {
                            if (std::get<KeywordToken>(*mIterator).getType() != KeywordToken::CATCH) {
                                throw AException{};
                            }
                        } catch (...) {
                            reportUnexpectedErrorAndSkip("expected catch keyword after try block");
                        }
                        ++mIterator;
                        parseCallArgs();
                        parseCodeBlock();

                        break;
                    }

                    default:
                        result << parseExpression();
                }

                break;
            }

            default:
                reportUnexpectedErrorAndSkip("code block expects identifier or semicolon");
                throw AException{};
        }
    }


    reportUnexpectedEof();
    return result;
}

_<ExpressionNode> Parser::parseExpression(RequiredPriority requiredPriority) {
    _<ExpressionNode> result = nullptr;
    for (; mIterator != mTokens.end(); ) {
        switch (mIterator->index()) {
            case got<KeywordToken>: {
                auto keyword = std::get<KeywordToken>(*mIterator);
                switch (keyword.getType()) {
                    case KeywordToken::THIS:
                        result = _new<ThisNode>();
                        ++mIterator;
                        break;

                    case KeywordToken::NULLPTR:
                        result = _new<NullptrNode>();
                        ++mIterator;
                        break;

                    case KeywordToken::CONST:
                    case KeywordToken::AUTO:
                        result = parseVariableDeclaration();
                        break;

                    case KeywordToken::TRUE:
                        result = _new<BoolNode>(true);
                        ++mIterator;
                        break;

                    case KeywordToken::FALSE:
                        result = _new<BoolNode>(false);
                        ++mIterator;
                        break;

                    case KeywordToken::USING:
                        // we are not interested in using, skip until ;
                        skipUntilSemicolonOrNewLine();
                        break;

                    default:
                        reportError("only auto, this, true, false and nullptr keywords are allowed in an expression");
                        throw AException{};
                }
                break;
            }
            case got<TernaryToken>: { // ternary operator
                result = parseTernary(result);
                break;
            }
            case got<IdentifierToken>: {
                auto name1 = std::get<IdentifierToken>(*mIterator).value();
                if (result == nullptr) {
                    // variable reference
                    result = parseIdentifier();
                } else {
                    // operator literal
                    result = _new<OperatorLiteralNode>(name1, result);
                    ++mIterator;
                }
                break;
            }

            case got<PlusToken>: {
                if (requiredPriority > RequiredPriority::LOW_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<BinaryPlusOperatorNode>(result, parseExpression());
            }

            case got<MinusToken>: {
                if (requiredPriority > RequiredPriority::LOW_PRIORITY) {
                    return result;
                }
                ++mIterator;
                if (result) {
                    return _new<BinaryMinusOperatorNode>(result, parseExpression());
                }
                return _new<UnaryMinusOperatorNode>(parseExpression());
            }

            case got<DoubleColonToken>: {
                // variable access or method call
                ++mIterator;
                if (result == nullptr) {
                    reportError("field access requires object");
                    throw AException{};
                }

                result = _new<StaticMemberAccessOperatorNode>(result, parseMemberAccess());
                break;
            }
            case got<PointerFieldAccessToken>:
            case got<FieldAccessToken>: {
                // variable access or method call
                ++mIterator;
                if (result == nullptr) {
                    reportError("field access requires object");
                    throw AException{};
                }

                result = _new<MemberAccessOperatorNode>(result, parseMemberAccess());

                break;
            }

            case got<StringToken>:
                result = _new<StringNode>(std::get<StringToken>(*mIterator).value());
                ++mIterator;
                break;

            case got<IntegerToken>: {
                auto& token = std::get<IntegerToken>(*mIterator);
                result = _new<IntegerNode>(token.value(), token.isHex());
                ++mIterator;
                break;
            }

            case got<FloatToken>:
                result = _new<FloatNode>(std::get<FloatToken>(*mIterator).value());
                ++mIterator;
                break;

            case got<LShiftToken>:
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<LShiftOperatorNode>(result, parseExpression());

            case got<RShiftToken>:
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<RShiftOperatorNode>(result, parseExpression());

            case got<LSquareBracketToken>: {
                if (result == nullptr) {
                    // lambda
                    result = parseLambda();
                } else {
                    // array style [] access
                    ++mIterator;
                    result = _new<ArrayAccessOperatorNode>(result, parseExpression());
                    expect<RSquareBracketToken>();
                    ++mIterator;
                }
                break;
            }

            case got<EqualToken>: { // assignment
                ++mIterator;
                return _new<AssignmentOperatorNode>(result, parseExpression());
            }

            case got<DoubleEqualToken>: { // == check operator
                ++mIterator;
                if (requiredPriority > RequiredPriority::COMPARE_OPERATORS) {
                    return result;
                }
                return _new<EqualsOperatorNode>(result, parseExpression());
            }

            case got<LAngleBracketToken>: { // < check operator
                ++mIterator;
                if (requiredPriority > RequiredPriority::COMPARE_OPERATORS) {
                    return result;
                }
                return _new<LessOperatorNode>(result, parseExpression());
            }

            case got<RAngleBracketToken>: { // > check operator
                ++mIterator;
                if (requiredPriority > RequiredPriority::COMPARE_OPERATORS) {
                    return result;
                }
                return _new<GreaterOperatorNode>(result, parseExpression());
            }

            case got<NotEqualToken>: { // != check operator
                ++mIterator;
                if (requiredPriority > RequiredPriority::COMPARE_OPERATORS) {
                    return result;
                }
                return _new<NotEqualsOperatorNode>(result, parseExpression());
            }

            case got<ModToken>: {
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<ModOperatorNode>(result, parseExpression());
            }

            case got<BitwiseOrToken>: {
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<BitwiseOrOperatorNode>(result, parseExpression());
            }

            case got<LogicalOrToken>: {
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<LogicalOrOperatorNode>(result, parseExpression());
            }

            case got<LogicalAndToken>: {
                if (requiredPriority > RequiredPriority::HIGH_PRIORITY) {
                    return result;
                }
                ++mIterator;
                return _new<LogicalAndOperatorNode>(result, parseExpression());
            }

            case got<LogicalNotToken>: {
                ++mIterator;
                if (result != nullptr) {
                    reportError("logical not is not a binary operator");
                    throw AException{};
                }
                result = _new<LogicalNotOperatorNode>(parseExpression(RequiredPriority::UNARY));
                break;
            }

            case got<AsteriskToken>: { // pointer dereference
                ++mIterator;
                result = _new<PointerDereferenceOperatorNode>(parseExpression(RequiredPriority::UNARY));
                break;
            }

            case got<AmpersandToken>: { // variable pointer creation
                ++mIterator;
                result = _new<PointerCreationOperatorNode>(parseExpression(RequiredPriority::UNARY));
                break;
            }

            case got<LCurlyBracketToken>: { // implicit initializer list initialization
                result = _new<ImplicitInitializerListCtorNode>(parseCurlyBracketsArgs());
                break;
            }

            default:
                if (result == nullptr) {
                    reportUnexpectedErrorAndSkip("not an expression");
                    throw AException{};
                }
                return result;

        }
    }
    return result;
}

_<ExpressionNode> Parser::parseTernary(const _<ExpressionNode>& condition) {
    expect<TernaryToken>();
    ++mIterator;
    auto onTrue = parseExpression();
    expect<ColonToken>();
    ++mIterator;
    auto onFalse = parseExpression();

    return _new<TernaryOperatorNode>(condition, onTrue, onFalse);
}

AVector<_<INode>> Parser::parseConstructorInitializerList() {
    if (!std::holds_alternative<ColonToken>(*mIterator)) {
        return {};
    }
    ++mIterator;
    AVector<_<INode>> result;

    for (; mIterator != mTokens.end();) {
        switch (mIterator->index()) {
            case got<IdentifierToken>: {
                auto fieldName = std::get<IdentifierToken>(*mIterator);
                ++mIterator;
                auto args = parseCallArgs();
                result << _new<OperatorCallNode>(fieldName.value(), args);
                break;
            }
            case got<CommaToken>:
                ++mIterator;
                break;

            default:
                return result;
        }
    }

    reportUnexpectedEof();
    throw AException{};
}

_<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    bool isConst = false, isStatic = false, isReference = false;
    AString typeName, variableName;
    int pointer = 0;

    // find type
    [&]{
        for (; mIterator != mTokens.end(); ++mIterator) {
            switch (mIterator->index()) {
                case got<KeywordToken>: {
                    switch (std::get<KeywordToken>(*mIterator).getType()) {
                        case KeywordToken::CONST:
                            isConst = true;
                            break;

                        case KeywordToken::STATIC:
                            isStatic = true;
                            break;

                        case KeywordToken::UNSIGNED:
                            typeName = "unsigned";
                            ++mIterator;
                            return;
                        case KeywordToken::AUTO:
                            typeName = "auto";
                            ++mIterator;
                            return;
                    }

                    break;
                }
                case got<IdentifierToken>: {
                    auto name = std::get<IdentifierToken>(*mIterator);
                    typeName = name.value();
                    ++mIterator;

                    if (mIterator->index() == got<LAngleBracketToken>) {
                        // template
                        ++mIterator;
                        switch (mIterator->index()) {
                            case got<IdentifierToken>:
                                typeName += "<" + parseTypename() + ">";
                                ++mIterator;
                                expect<RAngleBracketToken>();
                                break;

                            case got<RAngleBracketToken>: // diamond <>
                                break;

                            default:
                                reportUnexpectedErrorAndSkip("expected identifier or right angle bracket");
                        }
                        ++mIterator;
                    }

                    return;
                }

                default:
                    reportUnexpectedErrorAndSkip("expected identifier token");
                    throw AException{};
            }
        }
    }();

    // find name, pointer and reference modifiers
    for (; mIterator != mTokens.end(); ++mIterator) {
        switch (mIterator->index()) {
            case got<IdentifierToken>: {
                if (!variableName.empty()) {
                    reportUnexpectedErrorAndSkip("expected asterisk, ampersand or end of the variable definition");
                }
                variableName = std::get<IdentifierToken>(*mIterator).value();
                break;
            }

            case got<AsteriskToken>: { // pointer
                if (variableName.empty()) {
                    ++pointer;
                    break;
                }

                reportUnexpectedErrorAndSkip("expected end of the variable definition");
                throw AException{};
            }

            case got<AmpersandToken>: // reference
                if (variableName.empty()) {
                    isReference = true;
                    break;
                }

            default:
                if (variableName.empty()) {
                    reportUnexpectedErrorAndSkip("expected variable name");
                    throw AException{};
                }

                return _new<VariableDeclarationNode>(isConst, isStatic, typeName, variableName, pointer, isReference);
        }
    }

    reportUnexpectedEof();
    throw AException{};
}

void Parser::skipUntilSemicolonOrNewLine() {
    for (; mIterator != mTokens.end(); ++mIterator) {
        if (std::holds_alternative<SemicolonToken>(*mIterator) || std::holds_alternative<NewLineToken>(*mIterator)) {
            return;
        }
    }
}

bool Parser::parseUsing() {
    auto& nameToken = std::get<KeywordToken>(*mIterator);
    if (nameToken.getType() == KeywordToken::USING) {
        // using definition, skip it
        skipUntilSemicolonOrNewLine();
        return true;
    }
    return false;
}

void Parser::reportUnexpectedErrorAndSkip(const AString& string) {
    reportError(string + ", got "_as + getTokenName());
    skipUntilSemicolonOrNewLine();
}

AString Parser::getTokenName() {
    AString name;
    std::visit([&](auto&& arg) {
        name = arg.getName();
    }, *mIterator);
    return name;
}

void Parser::reportUnexpectedEof() {
    reportError("unexpected end of file");
    skipUntilSemicolonOrNewLine();
}

void Parser::reportError(const AString& message) {
    unsigned lineNumber = -1;
    if (mIterator < mTokens.end()) {
        std::visit([&](auto&& x) {
            lineNumber = x.getLineNumber();
        }, *mIterator);
    }


    ALogger::err(":" + (lineNumber == -1 ? "<last line>" : AString::number(lineNumber)) + " " + message);
    ++mErrorCount;
    if (mErrorCount > 10) {
        ALogger::err("Too many errors, terminating");
        throw Terminated{};
    }
}

_<ExpressionNode> Parser::parseMemberAccess() {
    switch (mIterator->index()) {
        case got<IdentifierToken>: {
            return parseIdentifier();
        }

        default:
            reportUnexpectedErrorAndSkip("expected identifier");
            throw AException{};
    }
    return nullptr;
}

_<ExpressionNode> Parser::parseLambda() {
    expect<LSquareBracketToken>();

    // skip everything until ]
    for (; mIterator->index() != got<RSquareBracketToken>; ++mIterator);
    ++mIterator;

    AVector<_<VariableDeclarationNode>> args;

    switch (mIterator->index()) {
        case got<LParToken>: {// lambda argument list
            args = parseFunctionDeclarationArgs();
            break;
        }

        case got<LCurlyBracketToken>:
            break;

        default:
            reportUnexpectedErrorAndSkip("expected left parenthesis or left curly bracket in lambda declaration");
    }
    auto code = parseCodeBlock();

    return _new<LambdaNode>(args, code);
}

_<ExpressionNode> Parser::parseIdentifier() {
    auto name1 = std::get<IdentifierToken>(*mIterator).value();
    ++mIterator;
    _<ExpressionNode> result;
    switch (mIterator->index()) {
        case got<LCurlyBracketToken>: {
            // initializer list object initialization
            result = _new<ExplicitInitializerListCtorNode>(name1, parseCurlyBracketsArgs());
            break;
        }

        case got<LAngleBracketToken>: {
            // template argument
            ++mIterator;
            auto templateArgument = parseTypename();
            expect<RAngleBracketToken>();
            ++mIterator;
            switch (mIterator->index()) {
                case got<LParToken>: {// template function call
                    auto callArgs = parseCallArgs();
                    result = _new<TemplateOperatorCallNode>(name1, callArgs, templateArgument);
                    break;
                }

                case got<DoubleColonToken>: { // typename
                    ++mIterator;
                    result = _new<StaticMemberAccessOperatorNode>(_new<TemplateOperatorTypenameNode>(templateArgument, name1),
                                                                  parseExpression());
                    break;
                }
            }
            break;
        } // fallthrough
        case got<LParToken>: {
            auto callArgs = parseCallArgs();
            result = _new<OperatorCallNode>(name1, callArgs);
            break;
        }

        default:
            result = _new<VariableReferenceNode>(name1);
            break;
    }
    return result;
}

unsigned Parser::getCurrentLineNumber() {
    if (mIterator >= mTokens.end()) {
        mIterator = mTokens.end() - 1;
    }
    unsigned lineNumber = 0;
    std::visit([&](auto&& v) {
        lineNumber = v.getLineNumber();
    }, *mIterator);
    return lineNumber;
}

_<INode> Parser::parseStructClassDefinition() {
    expect<IdentifierToken>();
    auto className = std::get<IdentifierToken>(*mIterator).value();
    AVector<_<INode>> items;
    ++mIterator;
    switch (mIterator->index()) {
        case got<SemicolonToken>: {
            // definition
            ++mIterator;
            break;
        }

        case got<ColonToken>: { // parents
            // skip it
            for (; mIterator->index() != got<LCurlyBracketToken>; ++mIterator) {}
        }

        case got<LCurlyBracketToken>: {
            // declaration
            ++mIterator;
            [&] {
                while (mIterator != mTokens.end()) {
                    switch (mIterator->index()) {
                        case got<RCurlyBracketToken>: { // close
                            ++mIterator;
                            return;
                        }
                        case got<IdentifierToken>: {
                            auto name1 = std::get<IdentifierToken>(*mIterator).value();
                            ++mIterator;
                            if (name1 == className) {
                                // class constructor
                                auto args = parseFunctionDeclarationArgs();
                                switch (mIterator->index()) {
                                    case got<ColonToken>:
                                    case got<LCurlyBracketToken>: {
                                        // declaration
                                        auto initializerList = parseConstructorInitializerList();
                                        auto code = parseCodeBlock();
                                        items << _new<ConstructorDeclarationNode>(className, className, args, className, initializerList, code);
                                        break;
                                    }
                                    case got<SemicolonToken>: {
                                        // definition
                                        // ignore it
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                        default:
                            ++mIterator;
                    }
                }
            }();

            switch (mIterator->index()) {
                case got<SemicolonToken>: {
                    ++mIterator;
                    break;
                }

                case got<IdentifierToken>: { // in-place variable declaration
                    ++mIterator; // ignore it
                    expect<SemicolonToken>();
                    ++mIterator;
                }
            }

            break;
        }
    }

    return _new<StructClassDefinition>(className, items);
}

AString Parser::parseTypename() {
    AString r;
    for (; mIterator < mTokens.end(); ++mIterator) {
        switch (mIterator->index()) {
            case got<IdentifierToken>:
                r += std::get<IdentifierToken>(*mIterator).value();
                break;

            case got<DoubleColonToken>:
                r += "::";
                break;

            case got<LAngleBracketToken>:
                r += parseTypename();
                expect<RCurlyBracketToken>();
                break;

            default: return r;
        }
    }
    return r;
}
