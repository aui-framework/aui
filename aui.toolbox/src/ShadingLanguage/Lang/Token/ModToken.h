//
// Created by alex2 on 6/17/2021.
//

#pragma once


#include "IToken.h"

class ModToken: public IToken {
public:
    ModToken();

    const char* getName() override;
};


