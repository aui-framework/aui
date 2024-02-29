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

#include "A2FingerTransformArea.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/AAngleRadians.h"

A2FingerTransformArea::A2FingerTransformArea() {
    setExpanding();
    setLayout(_new<AStackedLayout>());
}

void A2FingerTransformArea::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
    mTrackedPoints << TrackedPoint { .pos = event.position, .pointerIndex = event.pointerIndex };
}

void A2FingerTransformArea::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, event);
    auto trackedPoint = mTrackedPoints.findIf(event.pointerIndex, &TrackedPoint::pointerIndex);
    if (!trackedPoint) {
        return;
    }

    auto delta = pos - trackedPoint->pos;

    switch (mTrackedPoints.size()) {
        case 1: {
            if (glm::length2(delta) < 0.01f) {
                return;
            }
            trackedPoint->pos = pos;
            emit transformed(A2DTransform{ .offset = delta });
            break;
        }
        case 2: {
            auto calculateAngle = [&] {
                auto direction = mTrackedPoints.first().pos - mTrackedPoints.last().pos;
                return AAngleRadians(glm::atan(direction.x, direction.y));
            };
            auto calculateLength = [&] {
                return glm::distance(mTrackedPoints.first().pos, mTrackedPoints.last().pos);
            };

            auto prevLength = calculateLength();
            auto prevAngle = calculateAngle();
            trackedPoint->pos = pos;
            auto newLength = calculateLength();
            auto newAngle = calculateAngle();
            emit transformed(A2DTransform{ .offset = delta / 2.f, .rotation = prevAngle - newAngle, .scale = newLength / prevLength });
            break;   
        }
        default:
            break;
    }
}

void A2FingerTransformArea::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mTrackedPoints.removeAll(event.pointerIndex, &TrackedPoint::pointerIndex);
}

bool A2FingerTransformArea::consumesClick(const glm::ivec2& pos) {
    return true;
}
