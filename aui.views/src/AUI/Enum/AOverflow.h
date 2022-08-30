/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
