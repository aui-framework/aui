//
// Created by alex2 on 6/17/2021.
//

#pragma once

#include "IToken.h"

class BitwiseOrToken: public IToken {
public:
    BitwiseOrToken();

    const char* getName() override;
};


