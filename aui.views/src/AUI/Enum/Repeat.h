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
// Created by Alex2772 on 11/19/2021.
//

#pragma once


#include <AUI/Reflect/AEnumerate.h>

AUI_ENUM_FLAG(Repeat) {
        NONE = 0,
        X_Y = 0b11,
        X = 0b01,
        Y = 0b10,
};

AUI_ENUM_VALUES(Repeat, Repeat::NONE, Repeat::X, Repeat::Y, Repeat::X_Y)