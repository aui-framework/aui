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

#pragma once

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls screen orientation.
 * @ingroup ass
 */
enum class AScreenOrientation {
    UNDEFINED = 0,
    PORTRAIT = 1,
    LANDSCAPE = 2,
};

AUI_ENUM_VALUES(AScreenOrientation,
                AScreenOrientation::PORTRAIT,
                AScreenOrientation::LANDSCAPE)
