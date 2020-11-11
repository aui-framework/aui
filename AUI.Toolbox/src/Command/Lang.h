#pragma once

#include "ICommand.h"

class Lang: public ICommand {
public:
    ~Lang() override = default;

    AString getName() override;
    AString getSignature() override;
    AString getDescription() override;

    void run(Toolbox& t) override;
};