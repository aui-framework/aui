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
#include <AUI/Render/ABrush.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Util/ALayoutDirection.h>

namespace ass {

/**
 * @brief Gradient background.
 * @ingroup ass_properties
 * @details
 * Shape-aware equivalent of `BackgroundGradient`. Uses C++20 aggregate initialization.
 *
 * ```cpp
 * BackgroundGradientV2{ .gradient = ALinearGradientBrush{...}, .shape = AShapeRoundedRectangle{ 8_dp } }
 * ```
 */
struct BackgroundGradientV2 {
    AOptional<ALinearGradientBrush> gradient;
    AShape shape = AShapeRectangle{};
};

API_AUI_VIEWS Modifier operator|(Modifier thiz, BackgroundGradientV2 value);

} // namespace ass
