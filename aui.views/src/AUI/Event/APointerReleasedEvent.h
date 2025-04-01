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

/**
 * @brief Pointing method press event.
 */
struct APointerReleasedEvent {
    /**
     * @brief Where does the event occurred.
     */
    glm::vec2 position;

    /**
     * @brief Which button of the pointing device is triggered the event (AInput::LBUTTON if not present) or finger.
     */
    APointerIndex pointerIndex;

    /**
     * @brief Whether the pointer release event triggers click event or not.
     */
    bool triggerClick = true;


    /**
     * @brief pointerIndex treated as mouse button.
     * @details
     * = LBUTTON if finger event, pointerIndex.button() otherwise.
     */
    AInput::Key asButton = pointerIndex.button().valueOr(AInput::LBUTTON);
};