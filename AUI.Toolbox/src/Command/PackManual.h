//
// Created by alex2 on 5/31/2021.
//

#pragma once

#include <AUI/IO/APath.h>
#include "ICommand.h"

class PackManual: public ICommand {
public:
    AString getName() override;
    AString getSignature() override;
    AString getDescription() override;

    void run(Toolbox& t) override;
};