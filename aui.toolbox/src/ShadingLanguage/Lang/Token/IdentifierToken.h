//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "IToken.h"

/**
 * Token describing some name (name of function, variable, etc)
 */
class IdentifierToken: public IToken {
private:
    AString mName;

public:
    IdentifierToken(const AString& name) : mName(name) {}

    [[nodiscard]]
    const AString& value() const {
        return mName;
    }

    const char* getName() override {
        return "naming token";
    }
};


