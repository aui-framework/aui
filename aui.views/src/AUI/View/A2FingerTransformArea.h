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
 * The transformation data is emitted on delta basis via `transform` signal. This allows to easily incorporate
 * A2FingerTransformArea to transforming routines, including limit handling.
 *
 * Consider the following example, where the transformation is applied through ASS styles:
 * ```cpp
 * 
 * _<AView> multitouchDemo() {
 *   return _new<A2FingerTransformArea>() AUI_LET {
 *       it->setCustomStyle({
 *         MinSize { 256_dp },
 *         Border { 1_px, AColor::BLACK },
 *       });
 *       _<AView> blackRect = Stacked { _new<ALabel>("A2FingerTransformArea") AUI_WITH_STYLE {
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
 * ```
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
