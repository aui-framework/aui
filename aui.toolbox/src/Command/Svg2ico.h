#pragma once


#include "ICommand.h"

class Svg2ico: public ICommand {
public:
    AString getName() override;

    AString getSignature() override;

    AString getDescription() override;

    void run(Toolbox& t) override;
};


