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

#include "ATouchScroller.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/APlatform.h"
#include <glm/gtx/norm.hpp>

using namespace std::chrono;
using namespace std::chrono_literals;

void ATouchScroller::handlePointerPressed(const APointerPressedEvent& e) {
    AUI_ASSERTX(e.pointerIndex.isFinger(), "ATouchScroller is intended only for touchscreen events");
    mState = WaitingForThresholdState {
        .pointer = e.pointerIndex,
        .origin  = e.position,
    };
}

void ATouchScroller::handlePointerReleased(const APointerReleasedEvent& e) {
    AUI_ASSERTX(e.pointerIndex.isFinger(), "ATouchScroller is intended only for touchscreen events");
    if (auto s = std::get_if<ScrollingState>(&mState)) {
        auto direction = glm::normalize(s->currentVelocity);
        auto velocity = glm::max(glm::length(s->prevPrevVelocity),
                                 glm::length(s->prevVelocity),
                                 glm::length(s->currentVelocity));
        auto fps = static_cast<float>(AWindow::current()->getFps());
        mState = KineticScrollingState{
            .pointer = e.pointerIndex,
            .origin = s->origin,
            .velocity = velocity,
            .averageTimeDelta = 1.f / fps,
            .direction = direction,
        };
    }
}

glm::ivec2 ATouchScroller::handlePointerMove(glm::vec2 pos) {
    if (std::holds_alternative<std::nullopt_t>(mState)) {
        return {0, 0};
    }
    if (std::holds_alternative<KineticScrollingState>(mState)) {
        return {0, 0};
    }

    if (auto s = std::get_if<WaitingForThresholdState>(&mState)) {
        if (glm::distance2(glm::vec2(s->origin), glm::vec2(pos)) < std::pow(THRESHOLD.getValuePx(), 2)) {
            return {0, 0};
        }
        mState = ScrollingState{
            .pointer          = s->pointer,
            .origin           = s->origin,
            .previousPosition = s->origin,
        };
    }

    auto& s = std::get<ScrollingState>(mState);

    auto now = high_resolution_clock::now();
    s.timeBetweenFrames = duration_cast<microseconds>(now - s.lastFrameTime.valueOr(now));
    s.lastFrameTime = now;

    auto delta = s.previousPosition - pos;
    s.prevPrevVelocity = s.prevVelocity;
    s.prevVelocity = s.currentVelocity;
    s.currentVelocity = delta * INITIAL_ACCELERATION_COEFFICIENT;

    s.previousPosition = pos;
    
    return delta;
}

glm::ivec2 ATouchScroller::origin() const noexcept {
    return std::visit(aui::lambda_overloaded {
        [](const auto& r) -> glm::ivec2 {
            return r.origin;
        },
        [](std::nullopt_t) -> glm::ivec2 {
            AUI_ASSERTX(0, "ATouchScroller::origin is called in invalid state");
            return {0, 0};
        },
    }, mState);
}

AOptional<glm::ivec2> ATouchScroller::gatherKineticScrollValue() {
    auto s = std::get_if<KineticScrollingState>(&mState);
    if (!s) {
        return std::nullopt;
    }

    const auto now = high_resolution_clock::now();
    if (s->velocity < 0.001f) {
        return glm::ivec2{0, 0};
    }

    const auto timeDelta = duration_cast<microseconds>(now - s->lastFrameTime);
    s->lastFrameTime = now;
    glm::vec2 result = (s->velocity * s->averageTimeDelta) * s->direction;
    s->velocity -= s->averageTimeDelta * deceleration();
    return result;
}

float ATouchScroller::deceleration() {
    float scale;
    if (AWindow::current()) {
        scale = AWindow::current()->getDpiRatio();
    } else {
        scale = 1.0f;
    }
    return BASE_DECELERATION * scale;
}

