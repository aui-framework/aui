/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 07.11.2020.
//

#include <AUI/Common/AException.h>
#include "ALanguageCode.h"

ALanguageCode::ALanguageCode(const AString& str) {
    auto& ascii = str.bytes();
    if (str.length() != str.sizeBytes() || str.sizeBytes() != 5 || ascii[2] != '-' || islower(ascii[2]) || islower(ascii[3])) {
        throw AException("invalid language code: " + str);
    }
    mGroup[0] = ascii[0];
    mGroup[1] = ascii[1];
    mSubGroup[0] = ascii[3];
    mSubGroup[1] = ascii[4];
}

