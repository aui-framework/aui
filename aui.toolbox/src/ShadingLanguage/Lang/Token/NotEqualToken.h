//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "IToken.h"

class NotEqualToken: public IToken {
public:
    NotEqualToken();

    const char* getName() override;
};


