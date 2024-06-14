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
// Created by alex2 on 04.01.2021.
//

#pragma once

/**
 * @brief Controls the visibility of AView.
 * @ingroup ass
 * @ingroup views
 *
 * Enum         | Visibility | Consumes space | Consumes click
 * ---------------+------------+----------------+---------------
 * VISIBLE      | +          | +              | +
 * INVISIBLE    | -          | +              | +
 * UNREACHABLE  | +          | +              | -
 * GONE         | -          | -              | -
 */
enum class Visibility {

    /**
     * AView is visible and active
     */
    VISIBLE,

    /**
     * AView is invisible but still interacting to the mouse
     */
    INVISIBLE,

    /**
     * AView is visible but not interacting to the mouse
     */
    UNREACHABLE,

    /**
     * AView is invisible and does not interact with the mouse
     */
    GONE
};

AUI_ENUM_VALUES(Visibility, Visibility::VISIBLE, Visibility::INVISIBLE, Visibility::UNREACHABLE, Visibility::GONE)