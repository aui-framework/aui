//
// Created by alex2 on 6/12/2021.
//

#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AStringStream.h>
#include "SL.h"
#include "Lexer.h"
#include "Parser.h"

_<AST> aui::sl::parseCode(_<IInputStream> p) {
    Lexer l(std::move(p));
    auto parser = _new<Parser>(l.performLexAnalysis());
    return _new<AST>(parser->parse());
}

_<ExpressionNode> aui::sl::parseExpression(const AString& text) {
    Lexer l(_new<AStringStream>(text));
    auto parser = _new<Parser>(l.performLexAnalysis());
    return _<ExpressionNode>(parser->parseExpression());
}
