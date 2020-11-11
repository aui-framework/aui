#pragma once

#include "ICommand.h"

class Pack: public ICommand {
public:
    AString getName() override;
    AString getSignature() override;
    AString getDescription() override;

    void run(Toolbox& t) override;
};