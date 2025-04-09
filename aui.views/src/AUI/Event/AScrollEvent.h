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

#include <glm/glm.hpp>
#include "APointerIndex.h"

/**
 * @brief Pointing method scroll event.
 */
struct AScrollEvent {
    /**
     * @brief Where does the event occurred.
     * @details
     * For mouse, it's the position of the cursor.
     *
     * For touchscreen, it's the origin point where the scroll gesture started.
     */
    glm::vec2 origin;

    /**
     * @brief The amount of scroll
     * @details
     * This field is mutable in order to be consumable.
     *
     * By default, 120 is single mouse wheel click.
     * 120 = mouse scroll down, -120 = mouse scroll up.
     */
    mutable glm::vec2 delta;

    /**
     * @brief Whether the event simulated by kinetic scroll feature or not.
     */
    bool kinetic = false;

    /**
     * @brief Pointer index for touchscreen; LBUTTON for mouse cursor.
     */
    APointerIndex pointerIndex = APointerIndex::button(AInput::LBUTTON);
};
