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

#include <chrono>

#include <AUI/Common/AObject.h>
#include <AUI/Event/APointerPressedEvent.h>
#include <AUI/Event/APointerReleasedEvent.h>
#include <AUI/Event/AScrollEvent.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Animator/Curves/bezier.h>

/**
 * @brief Utility object that helps with touchscreen scroll events.
 * @ingroup views
 * @details
 * To use this class, you have to pass APointerPressedEvent APointerReleasedEvent to handlePointerPressed and handlePointerReleased
 * methods accordingly. To receive scroll value, use handlePointerMove method.
 */
class API_AUI_VIEWS ATouchScroller {
public:
    /*
     * @brief Deceleration for kinetic scroll
     * @details
     * Multiply by dpi ratio for proper work of kinetic scroll on all devices
     */
    static constexpr float BASE_DECELERATION = 1600.f;

    /**
     * @brief Initial acceleration after pointer released is defined by direction of pointer moving and this coefficient
     */
    static constexpr float INITIAL_ACCELERATION_COEFFICIENT = 60.f;

    static float deceleration();

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
    glm::ivec2 handlePointerMove(glm::vec2 pos);

    [[nodiscard]]
    glm::ivec2 origin() const noexcept;

    /**
     * @return std::nullopt if ATouchScroller is not in kinetic scroll state; value for kinetic
     * scroll otherwise.
     * @details
     * This method should be called every render frame in order to make continuous kinetic scroll.
     */
    [[nodiscard]]
    AOptional<glm::ivec2> gatherKineticScrollValue();

private:
    struct WaitingForThresholdState {
        APointerIndex pointer;
        glm::vec2 origin;
    };
    struct ScrollingState {
        APointerIndex pointer;
        glm::vec2 origin;
        glm::vec2 previousPosition = {0.f, 0.f}; // to calculate velocity
        glm::vec2 currentVelocity = {0.f, 0.f};
        glm::vec2 prevVelocity = {0.f, 0.f};
        glm::vec2 prevPrevVelocity = {0.f, 0.f};
        std::chrono::microseconds timeBetweenFrames;
        AOptional<std::chrono::high_resolution_clock::time_point> lastFrameTime;
    };

    struct KineticScrollingState {
        APointerIndex pointer;
        glm::vec2 origin;
        float velocity;
        float averageTimeDelta;
        glm::vec2 direction;

        std::chrono::high_resolution_clock::time_point lastFrameTime = std::chrono::high_resolution_clock::now();
    };

    std::variant<std::nullopt_t, WaitingForThresholdState, ScrollingState, KineticScrollingState> mState = std::nullopt;
};
