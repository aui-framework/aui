// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
