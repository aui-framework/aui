//
// Created by alex2 on 6/12/2021.
//

#pragma once


#include <cstdint>
#include "IToken.h"

/**
 * Token describing some integer constant
 */
class IntegerToken: public IToken {
private:
    int64_t mNumber;
    bool mIsHex;

public:
    IntegerToken(int64_t number, bool isHex) : mNumber(number), mIsHex(isHex) {}

    bool isHex() const {
        return mIsHex;
    }

    const char* getName() override;

    [[nodiscard]]
    int64_t value() const {
        return mNumber;
    }
};

