//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Util/ATokenizer.h>
#include <ShadingLanguage/Lang/Token/IToken.h>

#include "AnyToken.h"

class Lexer {
private:
    ATokenizer mTokenizer;

    void reportError(const AString& text);

public:
    explicit Lexer(_<IInputStream> is): mTokenizer(std::move(is)) {}


    AVector<AnyToken> performLexAnalysis();
};

