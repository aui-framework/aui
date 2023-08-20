//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once


#include "AUI/Common/AObject.h"
#include "AUI/Event/APointerPressedEvent.h"
#include "AUI/Event/APointerReleasedEvent.h"
#include "AMetric.h"

/**
 * @brief Utility object that helps with touchscreen scroll events.
 * @ingroup view
 * @details
 * To use this class, you have to pass APointerPressedEvent APointerReleasedEvent to handlePointerPressed and handlePointerReleased
 * methods accordingly. To receive scroll value, use handlePointerMove method.
 */
class ATouchScroller {
public:
    /**
     * @brief Distance that pointer have to pass in order to treat pointer move events as scroll events.
     */
    static constexpr AMetric THRESHOLD = 8_dp;

    /**
     * @brief Handles pointer pressed events.
     */
    void handlePointerPressed(const APointerPressedEvent& e);

    /**
     * @brief Handles pointer released events.
     */
    void handlePointerReleased(const APointerReleasedEvent& e);

    /**
     * @brief Handles pointer move events.
     * @param pos position of the event in relation to view that used ATouchScroller.
     * @return the amount of scroll in pixels.
     */
    glm::ivec2 handlePointerMove(glm::ivec2 pos);

private:
    struct WaitingForThresholdState {
        APointerIndex pointer;
        glm::ivec2 origin;
    };
    struct ScrollingState {
        APointerIndex pointer;
        glm::ivec2 origin;
        glm::ivec2 previousPosition; // to calculate velocity
    };

    std::variant<std::nullopt_t, WaitingForThresholdState, ScrollingState> mState;
};
