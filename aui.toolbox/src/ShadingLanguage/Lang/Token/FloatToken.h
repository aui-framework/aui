//
// Created by alex2 on 6/29/2021.
//

#pragma once

#include "IToken.h"

/**
 * Token describing some float constant
 */
class FloatToken: public IToken {
private:
    double mNumber;

public:
    FloatToken(double number): mNumber(number) {}

    const char* getName() override;

    [[nodiscard]]
    double value() const {
        return mNumber;
    }
};

