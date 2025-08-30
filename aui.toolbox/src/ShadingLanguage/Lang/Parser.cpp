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
#include "ShadingLanguage/Lang/AST/IndexedAttributesDeclarationNode.h"
#include "ShadingLanguage/Lang/AST/NonIndexedAttributesDeclarationNode.h"
#include "ShadingLanguage/Lang/AST/FlagDirectiveNode.h"
#include "Parser.h"
#include "AUI/Util/ARaiiHelper.h"
#include "Lexer.h"
#include "AUI/IO/AFileInputStream.h"
#include "ShadingLanguage/Lang/AST/ImportNode.h"
#include <range/v3/view.hpp>
#include <range/v3/algorithm.hpp>
#include <AUI/Traits/variant.h>


class Terminated {};


template<typename type>
constexpr size_t got = aui::variant::index_of<AnyToken, type>::value;

_<AST> Parser::parseShader() {
    AVector<_<INode>> nodes;
    try {
        for (; mIterator != mTokens.end(); [&] { if (mIterator != mTokens.end()) ++mIterator; }()) {
            try {
                // here we should find #includes and function definitions
                switch (mIterator->index()) {
                    case got<NewLineToken>:
                    case got<SemicolonToken>:
                        break;

                    case got<PreprocessorDirectiveToken>:
                        nodes << handlePreprocessor();
                        break;
                    case got<KeywordToken>: {
                        const auto keywordType = std::get<KeywordToken>(*mIterator).getType();
                        switch (keywordType) {
                            case KeywordToken::USING:
                                parseUsing();
                                break;

                            case KeywordToken::CLASS:
                            case KeywordToken::STRUCT: { // class or struct definition
                                ++mIterator;
                                nodes << parseStructClassDefinition();
                                break;
                            }

                            case KeywordToken::INPUT:
                            case KeywordToken::OUTPUT:
                                nodes << _new<IndexedAttributesDeclarationNode>(keywordType, parseIndexedAttributes());
                                break;

                            case KeywordToken::INTER:
                            case KeywordToken::UNIFORM:
                            case KeywordToken::TEXTURE:
                                nodes << _new<NonIndexedAttributesDeclarationNode>(keywordType, parseNonIndexedAttributes());
                                break;

                            case KeywordToken::ENTRY:
                                nextTokenAndCheckEof();
                                nodes << parseEntry();
                                break;
                            case KeywordToken::IMPORT:
                                if (!std::all_of(nodes.begin(), nodes.end(), [](const _<INode>& node) {
                                    return _cast<ImportNode>(node) != nullptr;
                                })) {
                                    reportUnexpectedErrorAndSkip("unexpected import: all imports must be in the beginning of the shader");
                                }
                                nodes << parseImportStatement();
                                break;

                            default:
                                #if defined(FMT_VERSION) && (FMT_VERSION < 100000)
                                reportUnexpectedErrorAndSkip("expected using, class, struct, input, output, inter, uniform, entry keywords"_format(keywordType));
                                #else
                                reportUnexpectedErrorAndSkip(fmt::format("expected using, class, struct, input, output, inter, uniform, entry keywords {}", keywordType));
                                #endif
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
                                        return _new<AST>(std::move(nodes));
                                    }

                                    default:
                                        reportUnexpectedErrorAndSkip("expected name token for constructor definition");
                                }

                                break;

                            case got<IdentifierToken>: {
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

                                    case got<LParToken>: {
                                        // function definition or declaration
                                        auto args = parseFunctionDeclarationArgs();

                                        if (mIterator->index() == got<LCurlyBracketToken>) {
                                            // function definition
                                            nodes << _new<FunctionDeclarationNode>(name1, name2, std::move(args), parseCodeBlock());
                                        } else {
                                            nodes << _new<FunctionDeclarationNode>(name1, name2, std::move(args), AVector<_<INode>>{});
                                        }
                                        break;
                                    }
                                    case got<DoubleColonToken>:
                                        // class constructor definition

                                        break;
                                }

                                break;
                            }

                            case got<EqualToken>: // assignment
                                nextTokenAndCheckEof();
                                nodes << _new<AssignmentOperatorNode>(_new<VariableReferenceNode>(name1),
                                                                      parseExpression());
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
    return _new<AST>(std::move(nodes));
}

IndexedAttributesDeclarationNode::Fields Parser::parseIndexedAttributes() {
    nextTokenAndCheckEof();
    expect<LCurlyBracketToken>();

    IndexedAttributesDeclarationNode::Fields result;

    for (;;) {
        nextTokenAndCheckEof();
        switch (mIterator->index()) {
            case got<NewLineToken>:
                break;

            case got<RCurlyBracketToken>:
                return result;

            case got<LSquareBracketToken>: {
                // indexed attribute declaration
                nextTokenAndCheckEof();
                const auto index = int(expect<IntegerToken>().value());
                if (result.contains(index)) {
                    reportError("index {} is already defined"_format(index));
                    skipUntilSemicolonOrNewLine();
                }
                nextTokenAndCheckEof();
                expect<RSquareBracketToken>();
                nextTokenAndCheckEof();
                result[index] = parseVariableDeclaration();
                break;
            }

            default:
                reportUnexpectedErrorAndSkip("expected attribute declaration");
        }
    }
}

NonIndexedAttributesDeclarationNode::Fields Parser::parseNonIndexedAttributes() {
    nextTokenAndCheckEof();
    expect<LCurlyBracketToken>();

    NonIndexedAttributesDeclarationNode::Fields result;

    for (;;) {
        nextTokenAndCheckEof();
        switch (mIterator->index()) {
            case got<NewLineToken>:
                break;

            case got<RCurlyBracketToken>:
                return result;

            case got<IdentifierToken>: {
                result.push_back(parseVariableDeclaration());
                break;
            }

            default:
                reportUnexpectedErrorAndSkip("expected attribute declaration");
        }
    }
}

void Parser::nextTokenAndCheckEof() {
    ++mIterator;
    if (mIterator == mTokens.end()) {
        reportError("unexpected <eof>");
    }
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
                auto name1 = std::get<IdentifierToken>(*mIterator).value();
                nextTokenAndCheckEof();
                if (mIterator->index() == got<IdentifierToken>) {
                    // variable definition
                    auto name2 = std::get<IdentifierToken>(*mIterator).value();

                    nextTokenAndCheckEof();
                    _<ExpressionNode> initializer;
                    if (mIterator->index() == got<EqualToken>) {
                        // with initializer
                        nextTokenAndCheckEof();
                        initializer = parseExpression();
                    }
                    result << _new<VariableDeclarationNode>(false,
                                                            false,
                                                            std::move(name1),
                                                            std::move(name2),
                                                            0,
                                                            false,
                                                            std::move(initializer));

                } else {
                    // whoops; it's an expression. rollback a little bit
                    --mIterator;
                    result << parseExpression();
                }
                break;
            }

            case got<NewLineToken>:
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

                    case KeywordToken::INPUT:
                    case KeywordToken::OUTPUT:
                    case KeywordToken::UNIFORM:
                    case KeywordToken::INTER:
                    case KeywordToken::TEXTURE:
                        mIterator--;
                        // fallthrough

                    default:
                        result << parseExpression();
                }

                break;
            }
            case got<PreprocessorDirectiveToken>:
                result << handlePreprocessor();
                break;

            default:
                reportUnexpectedErrorAndSkip("code block expects identifier or semicolon");
                throw AException{};
        }
    }


    reportUnexpectedEof();
    return result;
}

_<INode> Parser::handlePreprocessor() {
    auto& token = expect<PreprocessorDirectiveToken>();
    auto args = token.args();
    auto spaceIndex = args.find(" ");

    AString name = args.substr(0, spaceIndex);
    AString content = spaceIndex == std::string::npos ? "" : args.substr(spaceIndex);

    nextTokenAndCheckEof();

    return _new<FlagDirectiveNode>(std::move(name), std::move(content));
}

_<ExpressionNode> Parser::parseExpression() {
    _<ExpressionNode> temporaryValue; // storage for temporary non-binary nodes such ast constants, function calls, etc

    struct BinaryOperatorAndItsPriority {
        _<BinaryOperatorNode> op;
        int priority = -1;
    };

    AVector<BinaryOperatorAndItsPriority> binaryOperators;

    auto putValue = [&](_<ExpressionNode> node) {
        if (temporaryValue) {
            reportUnexpectedErrorAndSkip("temporaryValue is already set");
            throw AException{};
        }
        if (!binaryOperators.empty()) {
            if (binaryOperators.last().op->mRight) {
                reportUnexpectedErrorAndSkip("temporaryValue is already set");
                throw AException{};
            }
            binaryOperators.last().op->mRight = std::move(node);
            return;
        }
        temporaryValue = std::move(node);
    };

    auto takeValue = [&] {
        if (!temporaryValue) {
            reportUnexpectedErrorAndSkip("temporaryValue is empty");
            throw AException{};
        }
        auto v = std::move(temporaryValue);
        temporaryValue = nullptr; // to be sure
        return v;
    };

    enum class Priority {
        // to do last
        ASSIGNMENT,
        COMPARISON,
        BINARY_SHIFT,
        PLUS_MINUS,
        ASTERISK_SLASH,
        ARRAY_ACCESS,
        MEMBER_ACCESS,
        // to do first
    };

    auto handleBinaryOperator = [&]<aui::derived_from<BinaryOperatorNode> T>(Priority p) {
        nextTokenAndCheckEof();
        const int currentPriority = int(p);

        if (temporaryValue) {
            auto out = _new<T>(std::move(temporaryValue), nullptr);
            binaryOperators << BinaryOperatorAndItsPriority{
                .op = out,
                .priority = currentPriority,
            };
            AUI_ASSERT(temporaryValue == nullptr);
            return out;
        }

        for (const auto& o : binaryOperators | ranges::views::reverse) {
            if (o.priority < currentPriority && o.op->mRight) {
                // steal rhs
                auto currentOperator = _new<T>(std::move(o.op->mRight), nullptr);
                o.op->mRight = currentOperator;

                binaryOperators << BinaryOperatorAndItsPriority{
                    .op = currentOperator,
                    .priority = currentPriority,
                };
                return currentOperator;
            }
        }
        if (!binaryOperators.empty()) {
            auto root = std::min_element(binaryOperators.begin(), binaryOperators.end(), [](const BinaryOperatorAndItsPriority& lhs, const BinaryOperatorAndItsPriority& rhs) {
                return lhs.priority < rhs.priority;
            });
            auto out = _new<T>(root->op, nullptr);
            binaryOperators << BinaryOperatorAndItsPriority{
                    .op = out,
                    .priority = currentPriority,
            };
            AUI_ASSERT(temporaryValue == nullptr);
            return out;
        }

        reportUnexpectedErrorAndSkip("no left-hand statement");
        throw AException{};
    };

    auto handleUnaryOperator = [&]<aui::derived_from<ExpressionNode> T>() {

    };

    for (; mIterator != mTokens.end(); ) {
        switch (mIterator->index()) {
            case got<KeywordToken>: {
                auto keyword = std::get<KeywordToken>(*mIterator);
                switch (keyword.getType()) {
                    case KeywordToken::THIS:
                        putValue(_new<ThisNode>());
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::NULLPTR:
                        putValue(_new<NullptrNode>());
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::CONST:
                    case KeywordToken::AUTO:
                        putValue(parseVariableDeclaration());
                        break;

                    case KeywordToken::TRUE:
                        putValue(_new<BoolNode>(true));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::FALSE:
                        putValue(_new<BoolNode>(false));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::USING:
                        // we are not interested in using, skip until ;
                        skipUntilSemicolonOrNewLine();
                        break;

                    case KeywordToken::INPUT:
                        putValue(_new<VariableReferenceNode>("input"));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::UNIFORM:
                        putValue(_new<VariableReferenceNode>("uniform"));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::INTER:
                        putValue(_new<VariableReferenceNode>("inter"));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::OUTPUT:
                        putValue(_new<VariableReferenceNode>("output"));
                        nextTokenAndCheckEof();
                        break;

                    case KeywordToken::TEXTURE:
                        putValue(_new<VariableReferenceNode>("texture"));
                        nextTokenAndCheckEof();
                        break;

                    default:
                        reportError("only auto, this, true, false and nullptr keywords are allowed in an expression");
                        throw AException{};
                }
                break;
            }
            case got<TernaryToken>: { // ternary operator
                putValue(parseTernary(takeValue()));
                break;
            }
            case got<IdentifierToken>: {
                putValue(parseIdentifier());
                break;
            }

            case got<PlusToken>: {
                handleBinaryOperator.operator()<BinaryPlusOperatorNode>(Priority::PLUS_MINUS);
                break;
            }

            case got<MinusToken>: {
                handleBinaryOperator.operator()<BinaryMinusOperatorNode>(Priority::PLUS_MINUS);
                break;
            }

            case got<PointerFieldAccessToken>:
            case got<FieldAccessToken>: {
                // variable access or method call
                handleBinaryOperator.operator()<MemberAccessOperatorNode>(Priority::MEMBER_ACCESS);

                break;
            }

            case got<StringToken>:
                putValue(_new<StringNode>(std::get<StringToken>(*mIterator).value()));
                nextTokenAndCheckEof();
                break;

            case got<IntegerToken>: {
                auto& token = std::get<IntegerToken>(*mIterator);
                putValue(_new<IntegerNode>(token.value(), token.isHex()));
                nextTokenAndCheckEof();
                break;
            }

            case got<FloatToken>:
                putValue(_new<FloatNode>(std::get<FloatToken>(*mIterator).value()));
                nextTokenAndCheckEof();
                break;

            case got<LShiftToken>:
                nextTokenAndCheckEof();
                handleBinaryOperator.operator()<LShiftOperatorNode>(Priority::BINARY_SHIFT);
                break;

            case got<RShiftToken>:
                nextTokenAndCheckEof();
                handleBinaryOperator.operator()<RShiftOperatorNode>(Priority::BINARY_SHIFT);
                break;

            case got<LSquareBracketToken>: {
                // array style [] access
                auto arrayAccessNode = handleBinaryOperator.operator()<ArrayAccessOperatorNode>(Priority::ARRAY_ACCESS);
                AUI_ASSERT(arrayAccessNode->mRight == nullptr);
                arrayAccessNode->mRight = parseExpression();
                expect<RSquareBracketToken>();
                nextTokenAndCheckEof();
                break;
            }

            case got<EqualToken>: { // assignment
                handleBinaryOperator.operator()<AssignmentOperatorNode>(Priority::ASSIGNMENT);
                break;
            }

            case got<DoubleEqualToken>: { // == check operator
                handleBinaryOperator.operator()<EqualsOperatorNode>(Priority::COMPARISON);
                break;
            }

            case got<LAngleBracketToken>: { // < check operator
                handleBinaryOperator.operator()<LessOperatorNode>(Priority::COMPARISON);
                break;
            }

            case got<RAngleBracketToken>: { // > check operator
                handleBinaryOperator.operator()<GreaterOperatorNode>(Priority::COMPARISON);
                break;
            }

            case got<NotEqualToken>: { // != check operator
                handleBinaryOperator.operator()<NotEqualsOperatorNode>(Priority::COMPARISON);
                break;
            }

            case got<LogicalNotToken>: {
                handleUnaryOperator.operator()<LogicalNotOperatorNode>();
                break;
            }

            case got<AsteriskToken>: { // pointer dereference or multiply
                handleBinaryOperator.operator()<BinaryAsteriskOperatorNode>(Priority::ASTERISK_SLASH);
                break;
            }
            case got<DivideToken>: { // divide
                handleBinaryOperator.operator()<BinaryDivideOperatorNode>(Priority::ASTERISK_SLASH);
                break;
            }

            case got<LCurlyBracketToken>: { // implicit initializer list initialization
                putValue(_new<ImplicitInitializerListCtorNode>(parseCurlyBracketsArgs()));
                break;
            }

            case got<LParToken>: {
                nextTokenAndCheckEof();
                putValue(parseExpression());
                expect<RParToken>();
                nextTokenAndCheckEof();
                break;
            }


            default:
                if (temporaryValue && !binaryOperators.empty()) {
                    // should assign it to some operator
                    for (const auto& o : binaryOperators | ranges::views::reverse) {
                        if (o.op->mRight == nullptr) {
                            o.op->mRight = std::move(temporaryValue);
                            return binaryOperators.first().op;
                        }
                    }
                    reportUnexpectedErrorAndSkip("excess value");
                    throw AException{};
                }
                if (!binaryOperators.empty()) {
                    return ranges::min_element(binaryOperators, [](const auto& l, const auto& r) {
                        return l.priority <= r.priority;
                    })->op;
                }
                if (temporaryValue) {
                    return temporaryValue;
                }
                reportUnexpectedErrorAndSkip("not an expression");
                throw AException{};

        }
    }
    return temporaryValue;
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
                        default: break;
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
            if (name1.startsWith("SL_")) {
                reportError("'{}': 'SL_' prefix is reserved"_format(name1));
            }
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

_<INode> Parser::parseEntry() {
    return aui::ptr::manage_shared(new FunctionDeclarationNode("void", "entry", {}, parseCodeBlock()));
}

_<INode> Parser::parseImportStatement() {
    ++mIterator;
    auto& name = expect<IdentifierToken>();
    auto filename = mFileDir / "{}.sl"_format(name.value());
    try {
        Lexer l(_new<AFileInputStream>(filename));
        Parser p(l.performLexAnalysis(), mFileDir);
        return _new<ImportNode>(p.parseShader());
    } catch (const AIOException& e) {
        ALogger::err("Parser") << "Unable to read " << filename << ": " << e;
        reportUnexpectedErrorAndSkip("unable to read file, ignoring");
        throw;
    }
}
