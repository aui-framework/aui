//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "IToken.h"

class LShiftToken: public IToken {
public:
    LShiftToken() = default;

    const char* getName() override;
};


