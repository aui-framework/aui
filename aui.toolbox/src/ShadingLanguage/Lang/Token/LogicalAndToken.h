//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "IToken.h"

class LogicalAndToken: public IToken {
public:
    LogicalAndToken();

    const char* getName() override;
};


