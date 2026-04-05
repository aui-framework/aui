// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2025 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "AUI/Reflect/AEnumerate.h"

/**
 * Controls <code>AFuture::wait</code> behaviour.
 * @see AFuture::wait
 */
AUI_ENUM_FLAG(AFutureWait) {
    JUST_WAIT = 0b00,
    ALLOW_STACKFUL_COROUTINES = 0b10,

    /**
     * @brief Use work stealing.
     */
    ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP = 0b01,
    DEFAULT = ALLOW_STACKFUL_COROUTINES | ALLOW_TASK_EXECUTION_IF_NOT_PICKED_UP,
};

