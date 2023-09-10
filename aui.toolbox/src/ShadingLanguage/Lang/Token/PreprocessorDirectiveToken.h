//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "IToken.h"

class PreprocessorDirectiveToken: public IToken {
public:
    enum Type {
        INCLUDE,
        PRAGMA,
        IF,
        ENDIF,
        FLAG,
    };
private:
    Type mType;
    AString mArgs;

public:
    PreprocessorDirectiveToken(Type type, const AString& args) : mType(type), mArgs(args) {}

    const char* getName() override;

    [[nodiscard]]
    const AString& args() const noexcept {
        return mArgs;
    }

    static Type typeFromName(const AString& name);
};


