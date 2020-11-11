#pragma once

#include <Toolbox.h>

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual AString getName() = 0;
    virtual AString getSignature() = 0;
    virtual AString getDescription() = 0;

    virtual void run(Toolbox& t) = 0;

    void printHelp();
};