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

namespace ass {

/**
 * @brief Solid color background.
 * @ingroup ass_properties
 * @details
 * Unlike the legacy `BackgroundSolid`, this struct carries an explicit `shape` field
 * (defaulting to `AShapeRectangle{}`), so render callbacks do not need to pull
 * `borderRadius` from the view.  Uses C++20 aggregate initialization — no constructors.
 *
 * ```cpp
 * // rectangle
 * BackgroundSolidV2{ .color = AColor::RED }
 * // rounded rectangle
 * BackgroundSolidV2{ .color = AColor::RED, .shape = AShapeRoundedRectangle{ 8_dp } }
 * ```
 */
struct BackgroundSolidV2 {
    AColor color;
    AShape shape = AShapeRectangle{};
};

API_AUI_VIEWS Modifier operator|(Modifier thiz, BackgroundSolidV2 value);

} // namespace ass
