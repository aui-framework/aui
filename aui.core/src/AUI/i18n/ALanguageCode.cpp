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

#include "ALanguageCode.h"

#include <AUI/Common/AException.h>

ALanguageCode::ALanguageCode(const AString& str) {
    auto& ascii = str.bytes();
    if (str.sizeBytes() != 5) {
        throw AException("invalid language code length: " + str);
    }
    if (ascii[2] != '-' && ascii[2] != '_') {
        throw AException("invalid language code format: " + str);
    }
    mGroup[0] = tolower(ascii[0]);
    mGroup[1] = tolower(ascii[1]);
    mSubGroup[0] = toupper(ascii[3]);
    mSubGroup[1] = toupper(ascii[4]);
}

