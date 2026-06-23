//
// Created by alex2 on 6/12/2021.
//

#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AStringStream.h>
#include "SL.h"
#include "Lexer.h"
#include "Parser.h"

AArc<AST> aui::sl::parseCode(AArc<IInputStream> is, APath fileDir) {
    Lexer l(std::move(is));
    auto parser = _new<Parser>(l.performLexAnalysis(), std::move(fileDir));
    return parser->parseShader();
}

AArc<ExpressionNode> aui::sl::parseExpression(const AString& text) {
    Lexer l(_new<AStringStream>(text));
    auto parser = _new<Parser>(l.performLexAnalysis(), APath::workingDir());
    return AArc<ExpressionNode>(parser->parseExpression());
}

AVector<AArc<INode>> aui::sl::parseCodeBlock(const AString& text) {
    Lexer l(_new<AStringStream>(text));
    auto parser = _new<Parser>(l.performLexAnalysis(), APath::workingDir());
    return parser->parseCodeBlock();
}
