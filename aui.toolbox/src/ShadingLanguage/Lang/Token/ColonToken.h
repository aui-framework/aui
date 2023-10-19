//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include "IToken.h"

class ColonToken: public IToken {
public:
    ColonToken() = default;

    const char* getName() override;
};


