#pragma once

#include <AUI/Common/AStringVector.h>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AException.h>


class ICommand;

class IllegalArgumentsException: public AException {
public:
    IllegalArgumentsException(const AString& text) : AException(text) {}
};

struct Toolbox {
    Toolbox();
    ~Toolbox();
    AMap<AString, ICommand*> commands;
    AStringVector args;
};