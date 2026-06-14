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

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls how font is rendered.
 * @ingroup views
 */
enum class FontRendering {
    /**
     * @brief No antialiasing.
     * @details
     * Fastest, but results in jagged edges.
     */
    NEAREST = 0,

    /**
     * @brief Standard grayscale antialiasing.
     * @details
     * Good balance between speed and quality.
     */
    ANTIALIASING = 1,

    /**
     * @brief Subpixel antialiasing.
     * @details
     * - Best quality for LCD screens
     * - May require more processing power than other modes
     * - Particularly effective for small text sizes
     */
    SUBPIXEL = 2,
};

AUI_ENUM_VALUES(FontRendering, FontRendering::NEAREST, FontRendering::ANTIALIASING, FontRendering::SUBPIXEL)
