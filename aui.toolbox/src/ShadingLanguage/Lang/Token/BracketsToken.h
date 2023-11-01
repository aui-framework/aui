//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "IToken.h"

class LParToken: public IToken {
public:
    LParToken() = default;

    const char* getName() override;
};
class RParToken: public IToken {
public:
    RParToken() = default;

    const char* getName() override;
};
class LCurlyBracketToken: public IToken {
public:
    LCurlyBracketToken() = default;

    const char* getName() override;
};
class RCurlyBracketToken: public IToken {
public:
    RCurlyBracketToken() = default;

    const char* getName() override;
};

class LSquareBracketToken: public IToken {
public:
    LSquareBracketToken() = default;

    const char* getName() override;
};
class RSquareBracketToken: public IToken {
public:
    RSquareBracketToken() = default;

    const char* getName() override;
};

class LAngleBracketToken: public IToken {
public:
    LAngleBracketToken() = default;

    const char* getName() override;
};
class RAngleBracketToken: public IToken {
public:
    RAngleBracketToken() = default;

    const char* getName() override;
};