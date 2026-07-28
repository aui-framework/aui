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
#include "AUI/Reflect/AEnumerate.h"

/**
 * @brief Controls how fonts are rendered in the view.
 * @ingroup ass_properties
 * @details
 * When choosing a font rendering mode, consider:
 *
 * - The display type (LCD, OLED, etc.)
 * - Performance requirements
 * - Text size
 * - Visual quality requirements
 *
 * Note that subpixel rendering might not be suitable for all display types (particularly non-LCD displays found on
 * phones) and might produce color fringing artifacts in some cases.
 *
 */
enum class FontRendering {
    /**
     * @brief Use nearest pixel sampling.
     * @details
     * - The most basic rendering mode
     * - No anti-aliasing is applied
     * - Results in sharp but potentially jagged text edges
     * - Fastest performance but lowest visual quality
     * - Best suited for scenarios where performance is critical or when rendering small pixel-perfect fonts or
     *   stylistic conditions (e.g. Minecraft)
     */
    NEAREST = 0,

    /**
     * @brief Use anti-aliasing.
     * @details
     * - Applies standard anti-aliasing to text
     * - Smooths the edges of characters using grayscale pixels
     * - Provides better visual quality than NEAREST
     * - Good balance between quality and performance
     * - Suitable for most general use cases
     */
    ANTIALIASING = 1,

    /**
     * @brief Use subpixel rendering.
     * @details
     * - Most sophisticated rendering mode
     * - Uses RGB subpixels to improve horizontal resolution
     * - Provides the highest quality text rendering
     * - Best suited for LCD displays
     * - May require more processing power than other modes
     * - Particularly effective for small text sizes
     */
    SUBPIXEL = 2,
};

AUI_ENUM_VALUES(FontRendering, FontRendering::NEAREST, FontRendering::ANTIALIASING, FontRendering::SUBPIXEL)
