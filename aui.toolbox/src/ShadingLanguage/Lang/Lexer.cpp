//
// Created by alex2 on 6/12/2021.
//

#include <AUI/Logging/ALogger.h>
#include "Lexer.h"
#include <algorithm>
#include <ShadingLanguage/Lang/Token/FloatToken.h>
#include <AUI/Common/AChar.h>

AVector<AnyToken> Lexer::performLexAnalysis() {
    AVector<AnyToken> result;
    try {
        for (;;) {
            try {
                size_t sizeBefore = result.size();
                AChar c = mTokenizer.readChar();
                switch (c) {
                    case ' ':
                    case '\t':
                    case '\r':
                        // blank and unused shit
                        break;

                    case '!': {
                        if (mTokenizer.readChar() == '=') {
                            // not equal
                            result << NotEqualToken{};
                        } else {
                            // assignment or equal
                            result << LogicalNotToken{};
                            mTokenizer.reverseByte();
                        }
                        break;
                    }

                    case '&': {
                        if (mTokenizer.readChar() == '&') {
                            // binary and
                            result << LogicalAndToken{};
                        } else {
                            // reference, lambda capture
                            result << AmpersandToken{};
                            mTokenizer.reverseByte();
                        }
                        break;
                    }

                    case '=': {
                        if (mTokenizer.readChar() == '=') {
                            result << DoubleEqualToken{};
                        } else {
                            // assignment, lambda capture
                            result << EqualToken{};
                            mTokenizer.reverseByte();
                        }
                        break;
                    }

                    case '?': { // equality, lambda capture
                        result << TernaryToken{};
                        break;
                    }

                    case '.': {
                        result << FieldAccessToken{};
                        break;
                    }

                    case '+': {
                        result << PlusToken{};
                        break;
                    }

                    case '-': {
                        if (mTokenizer.readChar() == '>') {// ->
                            result << PointerFieldAccessToken{};
                        } else {
                            mTokenizer.reverseByte();
                            result << MinusToken{};
                        }
                        break;
                    }

                    case '*': {
                        result << AsteriskToken{};
                        break;
                    }

                    case ',': {
                        result << CommaToken{};
                        break;
                    }

                    case '(': {
                        result << LParToken{};
                        break;
                    }

                    case ')': {
                        result << RParToken{};
                        break;
                    }

                    case '{': {
                        result << LCurlyBracketToken{};
                        break;
                    }

                    case '}': {
                        result << RCurlyBracketToken{};
                        break;
                    }

                    case '[': {
                        result << LSquareBracketToken{};
                        break;
                    }

                    case ']': {
                        result << RSquareBracketToken{};
                        break;
                    }

                    case '<': {
                        if (mTokenizer.readChar() == '<') { // <<
                            result << LShiftToken{};
                        } else {
                            mTokenizer.reverseByte();
                            result << LAngleBracketToken{};
                        }
                        break;
                    }

                    case '>': {
                        if (mTokenizer.readChar() == '>') { // >>
                            result << RShiftToken{};
                        } else {
                            mTokenizer.reverseByte();
                            result << RAngleBracketToken{};
                        }
                        break;
                    }

                    case '"': {
                        // string
                        result << StringToken{mTokenizer.readStringUntilUnescaped('"')};
                        break;
                    }

                    case '#': {
                        // preprocessor directive
                        auto directiveName = mTokenizer.readString();
                        mTokenizer.readChar();
                        auto directiveArg = mTokenizer.readStringUntilUnescaped('\n');
                        directiveArg.erase(std::remove_if(directiveArg.begin(), directiveArg.end(), [](char16_t c) {
                            return c == '\r';
                        }), directiveArg.end());
                        result << PreprocessorDirectiveToken{
                                PreprocessorDirectiveToken::typeFromName(directiveName),
                                directiveArg};

                        break;
                    }

                    case ';': {
                        // COLON
                        result << SemicolonToken{};

                        break;
                    }

                    case '\n': {
                        // COLON
                        result << NewLineToken{};

                        break;
                    }

                    case '%': {
                        // mod
                        result << ModToken{};
                    }

                    case '|': {
                        if (mTokenizer.readChar() == '|') {
                            result << LogicalOrToken{};
                        } else {
                            result << BitwiseOrToken{};
                            mTokenizer.reverseByte();
                        }

                        break;
                    }

                    case ':': {
                        // COLON
                        if (mTokenizer.readChar() == ':') {
                            result << DoubleColonToken{};
                        } else {
                            mTokenizer.reverseByte();
                            result << ColonToken{};
                        }

                        break;
                    }

                    case '/': {
                        switch (mTokenizer.readChar()) {
                            case '/':
                                // single line comment, skip it
                                mTokenizer.skipUntilUnescaped('\n');
                                break;

                            case '*':
                                // multiline comment, skip until */
                                for (;;) {
                                    mTokenizer.skipUntil('*');
                                    if (mTokenizer.readChar() == '/') {
                                        break;
                                    }
                                }

                                break;
                            default:
                                // division
                                result << DivideToken{};
                                mTokenizer.reverseByte();
                        }
                        break;
                    }
                    default: {
                        if (c.alpha() || c == '_') {
                            // name
                            mTokenizer.reverseByte();
                            auto name = mTokenizer.readStringWhile([](char c) {
                                return AChar(c).alnum() || c == '_';
                            });
                            auto type = KeywordToken::fromName(name);
                            if (type == KeywordToken::NONE) {
                                result << IdentifierToken{name};
                            } else {
                                result << KeywordToken{type};
                            }
                        } else if (c.digit()) {
                            // number
                            mTokenizer.reverseByte();
                            auto[i, isHex] = mTokenizer.readUIntX();

                            {
                                // handle 2D and 3D keywords
                                mTokenizer.reverseByte();
                                auto n = mTokenizer.readChar();
                                if (n == 'D') {
                                    result << IdentifierToken{AString::number(i) + AString{'D'}};
                                    break;
                                }
                                mTokenizer.reverseByte();
                            }

                            auto n = mTokenizer.readChar();
                            if (n == '.') {
                                // float
                                AString d10 = mTokenizer.readStringWhile([](char c) -> bool { return isdigit(c); });
                                auto v = (AString::number(i) + "." + d10).toDoubleOrException();
                                result << FloatToken{v};

                                if (mTokenizer.readChar() != 'f') {
                                    mTokenizer.reverseByte();
                                }

                            } else {
                                mTokenizer.reverseByte();
                                result << IntegerToken{i, isHex};
                            }
                        } else {
                            reportError(
                                    "Invalid token '" + AString(1, c) + "' at " + AString::number(mTokenizer.getRow()) +
                                    ":" +
                                    AString::number(mTokenizer.getColumn()));
                        }
                    }
                }
                if (sizeBefore != result.size()) {
                    // feed with line number
                    std::visit([&](auto&& x) {
                        x.mLineNumber = mTokenizer.getRow();
                    }, result.last());
                }
            } catch (const AEOFException& e) {
                throw;
            } catch (const AException& e) {
                reportError(e.getMessage());
            }
        }
    } catch (const AEOFException& e) {}

    return result;
}

void Lexer::reportError(const AString& c) {
    ALogger::err(c);
    AString str = mTokenizer.readStringUntilUnescaped('\n');
    if (str.endsWith('\n')) {
        str.resize(str.length() - 1);
    }
    ALogger::warn(str);
    ALogger::warn("^");
    ALogger::warn("");
}
