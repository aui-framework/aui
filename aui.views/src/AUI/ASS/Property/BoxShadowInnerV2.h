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

#include <AUI/ASS/AShape.h>
#include <AUI/Util/Declarative/Modifier.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Common/AColor.h>

namespace ass {

/**
 * @brief Inner box shadow.
 * @ingroup ass_properties
 * @details
 * Shape-aware equivalent of `BoxShadowInner`. Uses C++20 aggregate initialization.
 *
 * ```cpp
 * BoxShadowInnerV2{ .offsetX = 0_dp, .offsetY = 2_dp, .blurRadius = 4_dp, .color = AColor::BLACK }
 * BoxShadowInnerV2{ .offsetX = 0_dp, .offsetY = 2_dp, .blurRadius = 4_dp, .color = AColor::BLACK,
 *                   .shape = AShapeRoundedRectangle{ 8_dp } }
 * ```
 */
struct BoxShadowInnerV2 {
    AMetric offsetX;
    AMetric offsetY;
    AMetric blurRadius;
    AMetric spreadRadius;
    AColor color;
    AShape shape = AShapeRectangle{};
};

API_AUI_VIEWS Modifier operator|(Modifier thiz, BoxShadowInnerV2 value);

}   // namespace ass
