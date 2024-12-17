/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2772 on 11/6/20.
//

#include "ICommand.h"

void ICommand::printHelp() {
    std::cout << getName() << " " << getSignature() << std::endl << "\t";
    size_t counter = 0;
    auto desc = getDescription().toStdString();
    for (auto i = desc.begin(); i != desc.end(); ++i, ++counter) {
        std::cout << (*i);
        if (counter == 80) {
            counter = 0;
            std::cout << std::endl << "\t";
        }
    }
    std::cout << std::endl;
}
