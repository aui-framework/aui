//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <AUI/Common/AString.h>
#include "IToken.h"

class StringToken: public IToken {
private:
    AString mContents;

public:
    StringToken(const AString& contents) : mContents(contents) {}

    [[nodiscard]]
    const AString& value() const {
        return mContents;
    }

    const char* getName() override;
};


