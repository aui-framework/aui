//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "IToken.h"

class LogicalOrToken: public IToken {
public:
    LogicalOrToken();

    const char* getName() override;
};


