//
// Created by alex2 on 6/12/2021.
//

#pragma once

class IToken {
friend class Lexer;
private:
    unsigned mLineNumber;

public:
    virtual ~IToken() = default;
    virtual const char* getName() = 0;

    [[nodiscard]]
    unsigned getLineNumber() const {
        return mLineNumber;
    }
};