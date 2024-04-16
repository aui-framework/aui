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

#include "AUI/View/AViewContainer.h"
#include "AView.h"
#include "AUI/Util/AAngleRadians.h"


struct A2DTransform {
    glm::vec2 offset = {0.f, 0.f};
    AAngleRadians rotation;
    float scale = 1.f;


    void applyDelta(const A2DTransform& other) {
        scale *= other.scale;
        offset += other.offset;
        rotation += other.rotation;
    }
};

/**
 * @brief Handles and processes the multitouch scale, transform and rotation events based on two-finger gestures.
 * @ingroup useful_views
 * @details
 * Represents a translucent area that handles and processes multi-finger transformation gestures (i.e. pinch-to-zoom,
 * move, rotate). A2FingerTransformArea is a container, so the transformed view should be somewhere inside.
 * A2FingerTransformArea does not apply any transformation though, so you can control transformation behaviour.
 *
 * The transformation data is emitted on delta basis via `transform` signal. This allows to easily incomporate
 * A2FingerTransformArea to transforming routines, including limit handling.
 *
 * Consider the following example, where the transformation is applied through ASS styles:
 * @code{cpp}
 * 
 * _<AView> multitouchDemo() {
 *   return _new<A2FingerTransformArea>() let {    
 *       it->setCustomStyle({
 *         MinSize { 256_dp },
 *         Border { 1_px, AColor::BLACK },
 *       });
 *       _<AView> blackRect = Stacked { _new<ALabel>("A2FingerTransformArea") with_style {
 *         FixedSize{200_dp, 100_dp},
 *         BackgroundSolid{AColor::BLACK},
 *         TextColor{AColor::WHITE},
 *         ATextAlign::CENTER,
 *       }};
 *       ALayoutInflater::inflate(it, Stacked { blackRect });
 *       connect(it->transformed, blackRect, [blackRect = blackRect.get(),
 *                                            keptTransform = _new<A2DTransform>()](const A2DTransform& transform) {
 *           keptTransform->applyDelta(transform);
 *           blackRect->setCustomStyle({
 *               TransformOffset{AMetric(keptTransform->offset.x, AMetric::T_PX),
 *                               AMetric(keptTransform->offset.y, AMetric::T_PX)},
 *               TransformScale{keptTransform->scale},
 *               TransformRotate{keptTransform->rotation},
 *           });
 *       });
 *   };
 * }
 * @endcode
 *
 * This example renders to the following result:
 * <img src="https://github.com/aui-framework/aui/raw/develop/docs/imgs/a2fingertransformarea.gif">
 */
class API_AUI_VIEWS A2FingerTransformArea: public AViewContainer
{
public:
    A2FingerTransformArea();

    void onPointerPressed(const APointerPressedEvent& event) override;
    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override;
    void onPointerReleased(const APointerReleasedEvent& event) override;

    bool consumesClick(const glm::ivec2& pos) override;

signals:
    emits<A2DTransform> transformed;

private:
    struct TrackedPoint {
        glm::vec2 pos;
        APointerIndex pointerIndex;
    };
    ASmallVector<TrackedPoint, 8> mTrackedPoints;

};
