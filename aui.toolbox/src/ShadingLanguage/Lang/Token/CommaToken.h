//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "IToken.h"

class CommaToken: public IToken {
public:
    const char* getName() override;
};


