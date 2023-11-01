//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "IToken.h"

class NewLineToken: public IToken {
public:
    NewLineToken() = default;

    const char* getName() override;
};


