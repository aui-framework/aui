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
// Created by alex2 on 5/31/2021.
//

#include "PackManual.h"
#include "Pack.h"
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Util/LZ.h>

void PackManual::run(Toolbox& t) {
    if (t.args.size() != 3)
    {
        throw IllegalArgumentsException("invalid argument count");
    }
    else
    {
        Pack::doPacking(t.args[0], t.args[1], t.args[2]);
    }
}

AString PackManual::getName() {
    return "pack_manual";
}

AString PackManual::getSignature() {
    return "<file to pack> <asset path for your application> <resulting cpp>";
}

AString PackManual::getDescription() {
    return "pack a file into the .cpp file with manually specifying asset path for your application";
}
