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

//
// Created by alex2 on 04.01.2021.
//

#pragma once

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls visibility of the overflowed contents of AView with AView::drawStencilMask.
 * @ingroup ass_properties
 */
enum class AOverflow {
    /**
     * @brief Overflowed contents are visible.
     */
    VISIBLE,

    /**
     * @brief Overflowed contents are hidden. Suitable for lists with scroll.
     * @details
     * On GPU accelerated renderer, HIDDEN does 2 extra draw calls thus decreasing performance a little bit.
     * Prefer VISIBLE if possible.
     */
    HIDDEN,

    /**
     * @brief Like HIDDEN, but view's ASS-styled background is also affected by mask.
     */
    HIDDEN_FROM_THIS,
};

/**
 * @brief Controls behavior of the overflowed text. Relevant to AAbstractLabel and its derivatives only.
 * @ingroup ass_properties
 */
enum class ATextOverflow {
    NONE,
    ELLIPSIS,
    CLIP
};

AUI_ENUM_VALUES(ATextOverflow,
                ATextOverflow::ELLIPSIS,
                ATextOverflow::CLIP)

/**
 * @brief Controls the behaviour of the default AView::drawStencilMask() implementation.
 * @ingroup ass_properties
 * @details
 * Controls how does the overflow mask is produced.
 *
 * Analogous to the -webkit-background-clip CSS rule.
 */
enum class AOverflowMask {
    /**
     * @brief Mask is produced from the (rounded) rect of the AView. The default value.
     */
    ROUNDED_RECT,

    /**
     * @brief Mask is produced from the alpha channel of the BackgroundImage.
     *
     * Helps in creating custom-shaped gradients, textures and effects.
     */
    BACKGROUND_IMAGE_ALPHA,
};

AUI_ENUM_VALUES(AOverflowMask,
                AOverflowMask::ROUNDED_RECT,
                AOverflowMask::BACKGROUND_IMAGE_ALPHA)
