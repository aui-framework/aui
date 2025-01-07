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

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls the visibility of AView.
 * @ingroup ass_properties
 */
AUI_ENUM_FLAG(Visibility) {
    /**
     * @brief If set, content of view is shown
     */
    FLAG_RENDER_NEEDED = 1 << 0,
    /**
     * @brief If set, view interacts to the mouse
     */
    FLAG_CONSUME_CLICKS = 1 << 1,
    /**
     * @brief If set, view occupies space in layout
     */
    FLAG_CONSUME_SPACE = 1 << 2,

    /**
     * @brief AView is visible and active
     */
    VISIBLE = FLAG_RENDER_NEEDED | FLAG_CONSUME_CLICKS | FLAG_CONSUME_SPACE,
    /**
     * @brief AView is invisible but still interacting to the mouse
     */
    INVISIBLE = FLAG_CONSUME_CLICKS | FLAG_CONSUME_SPACE,
    /**
     * @brief AView is visible but not interacting to the mouse
     */
    UNREACHABLE = FLAG_RENDER_NEEDED | FLAG_CONSUME_SPACE,
    /**
     * @brief AView is invisible, do not interacts to the mouse, but occupy some space in layout
     */
    CONSUME_SPACE_ONLY = FLAG_CONSUME_SPACE,
    /**
     * @brief AView is invisible and does not interact with the mouse
     */
    GONE = 0,
};

AUI_ENUM_VALUES(Visibility, Visibility::VISIBLE, Visibility::INVISIBLE, 
                            Visibility::UNREACHABLE, Visibility::CONSUME_SPACE_ONLY, 
                            Visibility::GONE)
