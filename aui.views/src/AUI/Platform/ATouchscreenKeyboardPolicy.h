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

/**
 * @brief Determines how window handles showing/hiding touchscreen
 */
enum class ATouchscreenKeyboardPolicy {
    /**
     * @brief Touchscreen keyboard will be requested if a view that got focused wants a keyboard and will be hidden otherwise
     * @note Default policy for touchscreen keyboard behaviour
     * @see AView::wantsTouchscreenKeyboard()
     */
    SHOWN_IF_NEEDED,
    /**
     * @brief If set, AUI will not send any show/hide touchscreen keyboard requests, user need to send these requests manually
     * @see ABaseWindow::requestShowTouchscreenKeyboard(), ABaseWindow::requestHideTouchscreenKeyboard()
     */
    MANUAL
};
