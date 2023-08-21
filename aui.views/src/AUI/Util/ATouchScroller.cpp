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

#include "ATouchScroller.h"
#include <glm/gtx/norm.hpp>

void ATouchScroller::handlePointerPressed(const APointerPressedEvent& e) {
    assert(("ATouchScroller is intended only for touchscreen events", e.pointerIndex.isFinger()));
    mState = WaitingForThresholdState {
        .pointer = e.pointerIndex,
        .origin  = e.position,
    };
}

void ATouchScroller::handlePointerReleased(const APointerReleasedEvent& e) {
    assert(("ATouchScroller is intended only for touchscreen events", e.pointerIndex.isFinger()));
    if (auto s = std::get_if<ScrollingState>(&mState)) {
        mState = KineticScrollingState{
            .pointer = e.pointerIndex,
            .origin = s->origin,
            .distance = glm::vec2(s->currentVelocity) * 60.f,
        };
    }
}

glm::ivec2 ATouchScroller::handlePointerMove(glm::ivec2 pos) {
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
    auto delta = s.previousPosition - pos;
    s.currentVelocity = delta;
    s.previousPosition = pos;
    
    return delta;
}

glm::ivec2 ATouchScroller::origin() const noexcept {
    return std::visit(aui::lambda_overloaded {
        [](const auto& r) -> glm::ivec2 {
            return r.origin;
        },
        [](std::nullopt_t) -> glm::ivec2 {
            assert((0, "ATouchScroller::origin is called in invalid state"));
            return {0, 0};
        },
    }, mState);
}

glm::ivec2 ATouchScroller::gatherKineticScrollValue() {
    using namespace std::chrono;

    if (auto s = std::get_if<KineticScrollingState>(&mState)) {
        const auto now = high_resolution_clock::now();
        const auto timeDelta = duration_cast<milliseconds>(now - s->beginTime);
        if (timeDelta >= DURATION) {
            return {0, 0};
        }
        auto newDistance = s->distance * s->curve(float(timeDelta.count()) / float(DURATION.count()));
        auto result = newDistance - s->prevDistance;
        s->prevDistance = newDistance;
        return result;
    }
    return {0, 0};
}

