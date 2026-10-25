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

#include <variant>

#include <glm/glm.hpp>

#include "AGestureEvent.h"
#include "APointerMoveEvent.h"
#include "APointerPressedEvent.h"
#include "APointerReleasedEvent.h"
#include "AScrollEvent.h"
#include "AUI/Common/AChar.h"
#include "AUI/Platform/AInput.h"

/**
 * @brief Pointer moved to `position`; see AView::onPointerMove.
 */
struct AInjectedPointerMove {
    glm::vec2 position;
    APointerMoveEvent event {};
};

/**
 * @brief Second press of a double click; see AView::onPointerDoubleClicked.
 */
struct AInjectedPointerDoubleClick {
    APointerPressedEvent event;
};

/**
 * @brief Gesture at `origin`; see AView::onGesture.
 */
struct AInjectedGesture {
    glm::ivec2 origin;
    AGestureEvent event;
};

/**
 * @brief Key press, auto-repeat or release; see AView::onKeyDown, AView::onKeyRepeat and AView::onKeyUp.
 */
struct AInjectedKey {
    enum class Action {
        DOWN,
        REPEAT,
        UP,
    };
    Action action;
    AInput::Key key;
};

/**
 * @brief Typed character; see AView::onCharEntered.
 */
struct AInjectedChar {
    AChar character;
};

/**
 * @brief An input event for ASurface::injectEvent.
 */
using AInjectedEvent = std::variant<APointerPressedEvent,
                                    APointerReleasedEvent,
                                    AInjectedPointerMove,
                                    AInjectedPointerDoubleClick,
                                    AScrollEvent,
                                    AInjectedGesture,
                                    AInjectedKey,
                                    AInjectedChar>;
