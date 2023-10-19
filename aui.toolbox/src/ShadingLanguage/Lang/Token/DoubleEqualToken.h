//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "IToken.h"

class DoubleEqualToken: public IToken {
public:
    DoubleEqualToken();

    const char* getName() override;
};


