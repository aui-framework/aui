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
#include "AUI/Event/AScrollEvent.h"
#include "AUI/Animator/Curves/linear.h"

/**
 * @brief Utility object that helps with touchscreen scroll events.
 * @ingroup view
 * @details
 * To use this class, you have to pass APointerPressedEvent APointerReleasedEvent to handlePointerPressed and handlePointerReleased
 * methods accordingly. To receive scroll value, use handlePointerMove method.
 */
class API_AUI_VIEWS ATouchScroller {
public:
    using AnimationCurve = aui::animation_curves::Linear;
    static constexpr auto DURATION = std::chrono::milliseconds(2500);

    /**
     * @brief Distance that pointer have to pass in order to treat pointer move events as scroll events.
     */
    static constexpr AMetric THRESHOLD = 8_dp;

    ATouchScroller() = default;

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

    [[nodiscard]]
    glm::ivec2 origin() const noexcept;

    /**
     * @return Value for kinetic scroll.
     * @details
     * This method should be called every render frame in order to make continuous kinetic scroll.
     */
    [[nodiscard]]
    glm::ivec2 gatherKineticScrollValue();

private:
    struct WaitingForThresholdState {
        APointerIndex pointer;
        glm::ivec2 origin;
    };
    struct ScrollingState {
        APointerIndex pointer;
        glm::ivec2 origin;
        glm::ivec2 previousPosition; // to calculate velocity
        glm::ivec2 previousPosition2 = glm::ivec2(0, 0);
    };

    struct KineticScrollingState {
        APointerIndex pointer;
        glm::ivec2 origin;
        glm::vec2 distance;
        glm::vec2 prevDistance = glm::vec2(0, 0);
        AnimationCurve curve;
        std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
    };

    std::variant<std::nullopt_t, WaitingForThresholdState, ScrollingState, KineticScrollingState> mState = std::nullopt;
};
