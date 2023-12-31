//
// Created by alex2 on 6/12/2021.
//

#include "BracketsToken.h"

const char* LParToken::getName() {
    return "left parentheses";
}

const char* RParToken::getName() {
    return "right parentheses";
}

const char* LCurlyBracketToken::getName() {
    return "left curly bracket";
}

const char* RCurlyBracketToken::getName() {
    return "right curly bracket";
}

const char* LSquareBracketToken::getName() {
    return "left square bracket";
}

const char* RSquareBracketToken::getName() {
    return "right square bracket";
}

const char* LAngleBracketToken::getName() {
    return "left angle bracket";
}

const char* RAngleBracketToken::getName() {
    return "right angle bracket";
}
