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
// Created by Alex2772 on 11/18/2021.
//

#pragma once


#include <span>
#include <variant>
#include <AUI/Common/AColor.h>
#include <AUI/Common/ASmallVector.h>
#include <AUI/Enum/Repeat.h>
#include "AUI/Enum/ImageRendering.h"
#include "AUI/Util/AAngleRadians.h"
#include "AUI/Traits/values.h"


/**
 * The simplest brush which used single solid color to fill.
 */
struct ASolidBrush {
    AColor solidColor = AColor::WHITE;
};


/**
 * The brush used with a custom graphics API shaders.
 */
struct ACustomShaderBrush {

};


/**
 * Brush which produces linear gradient by rectangle.
 */
struct ALinearGradientBrush {
    struct ColorEntry {
        aui::float_within_0_1 position;
        AColor color;
    };
    AVector<ColorEntry> colors;

    /**
     * @brief Clockwise gradient angle.
     * @details
     * When using an angle, 0_deg creates a vertical gradient running bottom to top, 90_deg creates a horizontal
     * gradient running left to right, and so on in a clockwise direction. Negative angles run in the counterclockwise
     * direction.
     */
    AAngleRadians rotation = 180_deg;
};

class ITexture;

/**
 * Brush which draws texture (raster image).
 */
struct ATexturedBrush {
    _<ITexture> texture;

    /**
     * Optional. Top left UV coords of the texture.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    AOptional<glm::vec2> uv1;

    /**
     * Optional. Bottom right UV coords of the texture.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    AOptional<glm::vec2> uv2;

    /**
     * Optional. Controls how does the image is scaled.
     * <dl>
     *     <dt><b>Warning!</b></dt>
     *     <dd>When set to <code>ImageRendering::SMOOTH</code>, on software renderer it produces extra CPU costs.</dd>
     * </dl>
     */
    ImageRendering imageRendering = ImageRendering::PIXELATED;

    /**
     * Optional. Controls how does the image behaves when it's size is not enough to cover the whole rect.
     */
    Repeat repeat = Repeat::NONE;
};


/**
 * Device-independent brush (shader) used for drawing.
 */
using ABrush = std::variant<ASolidBrush,
                            ALinearGradientBrush,
                            ATexturedBrush,
                            ACustomShaderBrush>;