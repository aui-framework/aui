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


#pragma once

#include <AUI/Reflect/AEnumerate.h>

/**
 * Determines which area of the view handles mouse (clicks and movement).
 */
enum class MouseCollisionPolicy {
    /**
     * Standard behaviour. Handle mouse only inside view's borders area.
     */
    DEFAULT,

    /**
     * Handle mouse inside view's margin area.
     */
    MARGIN,
};

AUI_ENUM_VALUES(MouseCollisionPolicy, MouseCollisionPolicy::DEFAULT, MouseCollisionPolicy::MARGIN)
