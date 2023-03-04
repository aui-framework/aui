//
// Created by alex2 on 6/12/2021.
//

#include <AUI/Common/AException.h>
#include "PreprocessorDirectiveToken.h"

PreprocessorDirectiveToken::Type PreprocessorDirectiveToken::typeFromName(const AString& name) {
    if (name == "include") {
        return INCLUDE;
    } else if (name == "if") {
        return IF;
    } else if (name == "endif") {
        return ENDIF;
    } else if (name == "pragma") {
        return PRAGMA;
    }
    throw AException("invalid preprocessor directive: " + name);
}

const char* PreprocessorDirectiveToken::getName() {
    return "preprocessor directive";
}
