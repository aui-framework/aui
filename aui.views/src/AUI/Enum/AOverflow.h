// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

//
// Created by alex2 on 04.01.2021.
//

#pragma once

#include <AUI/Reflect/AEnumerate.h>

/**
 * @brief Controls visibility of the overflowed contents of AView.
 * @ingroup ass
 * @ingroup views
 */
enum class AOverflow {
    VISIBLE,
    HIDDEN
};

/**
 * @brief Controls behavior of the overflowed text.
 * @ingroup ass
 * @ingroup views
 */
enum class TextOverflow {
    NONE,
    ELLIPSIS,
    CLIP
};

AUI_ENUM_VALUES(TextOverflow,
                TextOverflow::ELLIPSIS,
                TextOverflow::CLIP)

/**
 * @brief Controls the behaviour of the default AView::drawStencilMask() implementation.
 * @ingroup ass
 * @ingroup views
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
